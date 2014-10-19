#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
typedef struct Gem_OB gem;		// the strange order is so that managem_utils knows which gem type are we defining as "gem"
const int ACC=450;						// 450 is as good as 1000 up to reasonable numbers and takes half the time
#include "managem_utils.h"
typedef struct Gem_O gemO;
#include "leech_utils.h"


void worker(int len, int output_parens, int output_equations, int output_tree, int output_table, int output_debug, int output_info, int size)
{
	// utils compatibility
}

float gem_amp_power(gem gem1, gemO amp1)
{
	return (gem1.leech+4*0.23*2.8*amp1.leech)*gem1.bbound;		// yes, 4, because of 1.5 rescaling
}

int gem_alone_more_powerful(gem gem1, gem gem2, gemO amp2)
{
	return gem1.leech*gem1.bbound > gem_amp_power(gem2, amp2);
}

int gem_amp_more_powerful(gem gem1, gemO amp1, gem gem2, gemO amp2)
{
	return gem_amp_power(gem1, amp1) > gem_amp_power(gem2, amp2);
}

void print_amps_table(gem* gems, gemO* amps, float* spec_coeffs, int len)
{
	printf("# Gems\tManagem\tAmps\tPower (resc.)\tSpec coeff\n");
	int i;
	for (i=0;i<len;i++) printf("%d\t%d\t%d\t%.6f\t%.6lf\n", i+1, gem_getvalue(gems+i), gem_getvalue_O(amps+i), gem_amp_power(gems[i], amps[i]), spec_coeffs[i]);
	printf("\n");
}

void worker_omnia(int len, int lenc, int output_parens, int output_equations, int output_tree, int output_table, int output_debug, int output_info, int sizes, int sizec)
{
	printf("\n");
	int i;
	int size=sizes;
	gem* pool[len];
	int pool_length[len];
	pool[0]=malloc(2*sizeof(gem));
	gem_init(pool[0],  1, 1, 0);
	gem_init(pool[0]+1,1, 0, 1);
	pool_length[0]=2;

	for (i=1; i<len; ++i) {						// managem speccing
		int j,k,h,l;
		int eoc=(i+1)/2;				//end of combining
		int comb_tot=0;
		
		int grade_max=(int)(log2(i+1)+1);						// gems with max grade cannot be destroyed, so this is a max, not a sup
		gem* temp_pools[grade_max-1];								// get the temp pools for every grade
		int	temp_index[grade_max-1];								// index of work point in temp pools
		gem* subpools[grade_max-1];									// get subpools for every grade
		int	subpools_length[grade_max-1];
		for (j=0; j<grade_max-1; ++j) {							// init everything
			temp_pools[j]=malloc(size*sizeof(gem));
			temp_index[j]=0;
			subpools[j]=malloc(size*sizeof(gem));
			subpools_length[j]=1;
			gem_init(subpools[j],j+1,0,0);
		}
		for (j=0;j<eoc;++j) {												// combine gems and put them in temp pools
			if ((i-j)/(j+1) < 10) {										// value ratio < 10
				for (k=0; k< pool_length[j]; ++k) {
					for (h=0; h< pool_length[i-1-j]; ++h) {
						int delta=(pool[j]+k)->grade - (pool[i-1-j]+h)->grade;
						if (abs(delta)<=2) {								// grade difference <= 2
							comb_tot++;
							gem temp;
							gem_combine(pool[j]+k, pool[i-1-j]+h, &temp);
							int grd=temp.grade-2;
							temp_pools[grd][temp_index[grd]]=temp;
							temp_index[grd]++;
							if (temp_index[grd]==size) {									// let's skim a pool
								int length=size+subpools_length[grd];
								gem* temp_array=malloc(length*sizeof(gem));
								int index=0;
								for (l=0; l<temp_index[grd]; ++l) {					// copy new gems
									temp_array[index]=temp_pools[grd][l];
									index++;
								}
								temp_index[grd]=0;				// temp index reset
								for (l=0; l<subpools_length[grd]; ++l) {		// copy old gems
									temp_array[index]=subpools[grd][l];
									index++;
								}
								free(subpools[grd]);		// free
								gem_sort(temp_array,length);								// work starts
								
								int broken=0;
								float lim_bbound=-1;
								for (l=length-1;l>=0;--l) {
									if ((int)(ACC*temp_array[l].bbound)<=(int)(ACC*lim_bbound)) {
										temp_array[l].grade=0;
										broken++;
									}
									else lim_bbound=temp_array[l].bbound;
								}													// all unnecessary gems destroyed
								
								subpools_length[grd]=length-broken;
								subpools[grd]=malloc(subpools_length[grd]*sizeof(gem));		// pool init via broken
								
								index=0;
								for (l=0; l<length; ++l) {			// copying to subpool
									if (temp_array[l].grade!=0) {
										subpools[grd][index]=temp_array[l];
										index++;
									}
								}
								free(temp_array);			// free
							}												// rebuilt subpool[grd], work restarts
						}
					}
				}
			}
		}
		int grd;
		for (grd=0; grd<grade_max-1; ++grd)	{									// let's put remaining gems on
			if (temp_index[grd] != 0) {
				int length=temp_index[grd]+subpools_length[grd];
				gem* temp_array=malloc(length*sizeof(gem));
				int index=0;
				for (l=0; l<temp_index[grd]; ++l) {										// copy new gems
					temp_array[index]=temp_pools[grd][l];
					index++;
				}
				for (l=0; l<subpools_length[grd]; ++l) {		// copy old gems
					temp_array[index]=subpools[grd][l];
					index++;
				}
				free(subpools[grd]);		// free
				gem_sort(temp_array,length);								// work starts
				int broken=0;
				float lim_bbound=-1;
				for (l=length-1;l>=0;--l) {
					if ((int)(ACC*temp_array[l].bbound)<=(int)(ACC*lim_bbound)) {
						temp_array[l].grade=0;
						broken++;
					}
					else lim_bbound=temp_array[l].bbound;
				}													// all unnecessary gems destroyed
				subpools_length[grd]=length-broken;
				subpools[grd]=malloc(subpools_length[grd]*sizeof(gem));		// pool init via broken
				index=0;
				for (l=0; l<length; ++l) {			// copying to subpool
					if (temp_array[l].grade!=0) {
						subpools[grd][index]=temp_array[l];
						index++;
					}
				}
				free(temp_array);			// free
			}												// subpool[grd] is now full
		}
		pool_length[i]=0;
		for (grd=0; grd<grade_max-1; ++grd) pool_length[i]+=subpools_length[grd];
		pool[i]=malloc(pool_length[i]*sizeof(gem));

		int place=0;
		for (grd=0;grd<grade_max-1;++grd) {			// copying to pool
			for (j=0; j<subpools_length[grd]; ++j) {
				pool[i][place]=subpools[grd][j];
				place++;
			}
		}
		for (grd=0;grd<grade_max-1;++grd) {		 // free
			free(temp_pools[grd]);
			free(subpools[grd]);
		}

		printf("Managem spec: %d\n",i+1);
		if (output_info) {
			printf("Total raw:\t%d\n",comb_tot);
			printf("Average raw:\t%d\n",comb_tot/(grade_max-1));
			printf("Pool:\t%d\n\n",pool_length[i]);
		}
		fflush(stdout);
	}
	printf("Gem speccing done!\n\n");
	
	size=sizec;
	gem* poolc[lenc];
	int poolc_length[lenc];
	poolc[0]=malloc(sizeof(gem));	// grade leech bbound
	gem_init(poolc[0],1,1,1);			// start gem does not matter
	poolc_length[0]=1;

	for (i=1; i<lenc; ++i) {				// managem combining
		int j,k,h,l;
		int eoc=(i+1)/2;				//end of combining
		int comb_tot=0;
		
		int grade_max=(int)(log2(i+1)+1);						// gems with max grade cannot be destroyed, so this is a max, not a sup
		gem* temp_pools[grade_max-1];								// get the temp pools for every grade
		int	temp_index[grade_max-1];								// index of work point in temp pools
		gem* subpools[grade_max-1];									// get subpools for every grade
		int	subpools_length[grade_max-1];
		for (j=0; j<grade_max-1; ++j) {							// init everything
			temp_pools[j]=malloc(size*sizeof(gem));
			temp_index[j]=0;
			subpools[j]=malloc(size*sizeof(gem));
			subpools_length[j]=1;
			gem_init(subpools[j],j+1,0,0);
		}
		for (j=0;j<eoc;++j) {												// combine gems and put them in temp pools
			if ((i-j)/(j+1) < 10) {										// value ratio < 10
				for (k=0; k< poolc_length[j]; ++k) {
					for (h=0; h< poolc_length[i-1-j]; ++h) {
						int delta=(poolc[j]+k)->grade - (poolc[i-1-j]+h)->grade;
						if (abs(delta)<=2) {								// grade difference <= 2
							comb_tot++;
							gem temp;
							gem_combine(poolc[j]+k, poolc[i-1-j]+h, &temp);
							int grd=temp.grade-2;
							temp_pools[grd][temp_index[grd]]=temp;
							temp_index[grd]++;
							if (temp_index[grd]==size) {									// let's skim a pool
								int length=size+subpools_length[grd];
								gem* temp_array=malloc(length*sizeof(gem));
								int index=0;
								for (l=0; l<temp_index[grd]; ++l) {					// copy new gems
									temp_array[index]=temp_pools[grd][l];
									index++;
								}
								temp_index[grd]=0;				// temp index reset
								for (l=0; l<subpools_length[grd]; ++l) {		// copy old gems
									temp_array[index]=subpools[grd][l];
									index++;
								}
								free(subpools[grd]);		// free
								gem_sort(temp_array,length);								// work starts
								
								int broken=0;
								float lim_bbound=-1;
								for (l=length-1;l>=0;--l) {
									if ((int)(ACC*temp_array[l].bbound)<=(int)(ACC*lim_bbound)) {
										temp_array[l].grade=0;
										broken++;
									}
									else lim_bbound=temp_array[l].bbound;
								}													// all unnecessary gems destroyed
								
								subpools_length[grd]=length-broken;
								subpools[grd]=malloc(subpools_length[grd]*sizeof(gem));		// pool init via broken
								
								index=0;
								for (l=0; l<length; ++l) {			// copying to subpool
									if (temp_array[l].grade!=0) {
										subpools[grd][index]=temp_array[l];
										index++;
									}
								}
								free(temp_array);			// free
							}												// rebuilt subpool[grd], work restarts
						}
					}
				}
			}
		}
		int grd;
		for (grd=0; grd<grade_max-1; ++grd)	{									// let's put remaining gems on
			if (temp_index[grd] != 0) {
				int length=temp_index[grd]+subpools_length[grd];
				gem* temp_array=malloc(length*sizeof(gem));
				int index=0;
				for (l=0; l<temp_index[grd]; ++l) {										// copy new gems
					temp_array[index]=temp_pools[grd][l];
					index++;
				}
				for (l=0; l<subpools_length[grd]; ++l) {		// copy old gems
					temp_array[index]=subpools[grd][l];
					index++;
				}
				free(subpools[grd]);		// free
				gem_sort(temp_array,length);								// work starts
				int broken=0;
				float lim_bbound=-1;
				for (l=length-1;l>=0;--l) {
					if ((int)(ACC*temp_array[l].bbound)<=(int)(ACC*lim_bbound)) {
						temp_array[l].grade=0;
						broken++;
					}
					else lim_bbound=temp_array[l].bbound;
				}													// all unnecessary gems destroyed
				subpools_length[grd]=length-broken;
				subpools[grd]=malloc(subpools_length[grd]*sizeof(gem));		// pool init via broken
				index=0;
				for (l=0; l<length; ++l) {			// copying to subpool
					if (temp_array[l].grade!=0) {
						subpools[grd][index]=temp_array[l];
						index++;
					}
				}
				free(temp_array);			// free
			}												// subpool[grd] is now full
		}
		poolc_length[i]=0;
		for (grd=0; grd<grade_max-1; ++grd) poolc_length[i]+=subpools_length[grd];
		poolc[i]=malloc(poolc_length[i]*sizeof(gem));

		int place=0;
		for (grd=0;grd<grade_max-1;++grd) {			// copying to pool
			for (j=0; j<subpools_length[grd]; ++j) {
				poolc[i][place]=subpools[grd][j];
				place++;
			}
		}
		for (grd=0;grd<grade_max-1;++grd) {		 // free
			free(temp_pools[grd]);
			free(subpools[grd]);
		}

		printf("Managem comb: %d\n",i+1);
		if (output_info) {
			printf("Total raw:\t%d\n",comb_tot);
			printf("Average raw:\t%d\n",comb_tot/(grade_max-1));
			printf("Pool:\t%d\n\n",poolc_length[i]);
		}
		fflush(stdout);
	}
	printf("Gem combining done!\n\n");

	int lena;
	if (lenc > 2*len) lena=lenc;	// see which is bigger between 2x spec len and comb len
	else lena=2*len;							// and we'll combspec till there
	
	gemO* poolO[lena]; 
	int poolO_length[lena];
	poolO[0]=malloc(sizeof(gemO));
	gem_init_O(poolO[0],1,1);
	poolO_length[0]=1;

	for (i=1; i<lena; ++i) {			// amps computing
		int j,k,h;
		int grade_max=(int)(log2(i+1)+1);		// gems with max grade cannot be destroyed, so this is a max, not a sup
		poolO_length[i]=grade_max-1;
		poolO[i]=malloc(poolO_length[i]*sizeof(gemO));
		for (j=0; j<poolO_length[i]; ++j) gem_init_O(poolO[i]+j,j+2,1);
		int eoc=(i+1)/2;				//end of combining
		int comb_tot=0;

		for (j=0;j<eoc;++j) {										// combine and put istantly in right pool
			if ((i-j)/(j+1) < 10) {										// value ratio < 10
				for (k=0; k< poolO_length[j]; ++k) {
					for (h=0; h< poolO_length[i-1-j]; ++h) {
						int delta=(poolO[j]+k)->grade - (poolO[i-1-j]+h)->grade;
						if (abs(delta)<=2) {								// grade difference <= 2
							comb_tot++;
							gemO temp;
							gem_combine_O(poolO[j]+k, poolO[i-1-j]+h, &temp);
							int grd=temp.grade-2;
							if (gem_better(temp, poolO[i][grd])) {
								poolO[i][grd]=temp;
							}
						}
					}
				}
			}
		}

		printf("Amplifier: %d\n",i+1);
		if (output_info) {
			printf("Raw:\t%d\n",comb_tot);
			printf("Pool:\t%d\n\n",poolO_length[i]);
		}
	}
	printf("Amplifier pooling done!\n\n");			// compression?
	
	int j,k,h,l,m;								// let's choose the right gem-amp combo
	gem gems[len];								// for every speccing value
	gemO amps[len];								// we'll choose the best amps
	gem gemsc[len];								// and the best NC combine
	gemO ampsc[len];							// for both;
	float sc_coeffs[len];
	gem_init(gems,1,1,0);
	gem_init_O(amps,0,0);
	gem_init(gemsc,1,0,0);
	gem_init_O(ampsc,0,0);
	sc_coeffs[0]=0;
	printf("Gem:\n");
	gem_print(gems);
	printf("Amplifier:\n");
	gem_print_O(amps);
	
	for (i=1;i<len;++i) {																	// for every gem value
		gem_init(gems+i,0,0,0);															// we init the gems
		gem_init_O(amps+i,0,0);															// to extremely weak ones
		gem_init(gemsc+i,0,0,0);
		gem_init_O(ampsc+i,0,0);
		sc_coeffs[i]=0;
		for (j=-1;j<2*i+2;++j) {														// for every amp value from 0 to to 2*gem_value
			//int NS=(i+1)+6*(j+1);															// we get the num of gems used in speccing
			//float c = -log(NS)/log(lenc);											// we compute the combination number (yes, it's negative)
			float c=1;
			for (k=0;k<pool_length[i];++k) {									// then we search in the gem pool
				for (l=0; l<poolc_length[lenc-1]; ++l) {					// and in the NC gem comb pool
					for (m=0; m<poolO_length[lenc-1]; ++m) {				// aaand in the NC amp pool
						if (j==-1) {																// if no amp is needed we compare the gem alone
							float sc_coeff=pow(gem_power(poolc[lenc-1][l]), c)*gem_power(pool[i][k]);
							if (sc_coeff>sc_coeffs[i]) {
								sc_coeffs[i]=sc_coeff;
								gems[i]=pool[i][k];
								gem_init_O(amps+i,0,0);
								gemsc[i]=poolc[lenc-1][l];
								gem_init_O(ampsc+i,0,0);
							}
						}
						else for (h=0;h<poolO_length[j];++h) {			// else we look in the amp pool and compare
							float sc_coeff=pow(poolc[lenc-1][l].bbound, c)*pool[i][k].bbound*(pow(poolc[lenc-1][l].leech, c)*pool[i][k].leech+2.576*pow(poolO[lenc-1][m].leech, c)*pool[j][h].leech);
							if (sc_coeff>sc_coeffs[i]) {
								sc_coeffs[i]=sc_coeff;
								gems[i]=pool[i][k];
								amps[i]=poolO[j][h];
								gemsc[i]=poolc[lenc-1][l];
								ampsc[i]=poolO[lenc-1][m];
							}
						}
					}
				}
			}
		}
		printf("Managem spec\n");
		printf("Value:\t%d\n",i+1);
		if (output_info) printf("Pool:\t%d\n",pool_length[i]);
		gem_print(gems+i);
		printf("Amplifier spec\n");
		printf("Value:\t%d\n",gem_getvalue_O(amps+i));
		if (output_info) printf("Pool:\t%d\n",poolO_length[gem_getvalue_O(amps+i)-1]);
		gem_print_O(amps+i);
		printf("Managem combine\n");
		printf("Comb:\t%d\n",lenc);
		if (output_info) printf("Pool:\t%d\n",poolc_length[lenc-1]);
		gem_print(gemsc+i);
		printf("Amplifier combine\n");
		printf("Comb:\t%d\n",lenc);
		if (output_info) printf("Pool:\t%d\n",poolO_length[lenc-1]);
		gem_print_O(ampsc+i);
		printf("Global power (resc.):\t%f\n", gem_amp_power(gems[i], amps[i]));
		printf("Spec-combine coefficient:\t%f\n\n", sc_coeffs[i]);
		fflush(stdout);								// forces buffer write, so redirection works well
	}
printf("pippo");
	if (output_parens) {
		printf("Managem combining scheme:\n");
		print_parens(gems+len-1);
		printf("\n\n");
		printf("Amplifier combining scheme:\n");
		print_parens_O(amps+len-1);
		printf("\n\n");
	}
	if (output_tree) {
		printf("Managem tree:\n");
		print_tree(gems+len-1, "");
		printf("\n");
		printf("Amplifier tree:\n");
		print_tree_O(amps+len-1, "");
		printf("\n");
	}
	if (output_table) print_amps_table(gems, amps, sc_coeffs, len);

	if (output_debug) {
		printf("Printing all parens for every best setup:\n\n");
		for (i=2;i<len;++i) {
			printf("Total value:\t%d\n\n",i+1);
			printf("Managem combining scheme:\n");
			print_parens(gems+i-1);
			printf("\n\n");
			printf("Amplifier combining scheme:\n");
			print_parens_O(amps+i-1);
			printf("\n\n\n");
		}
	}
	if (output_equations) {		// it ruins gems, must be last
		printf("Managem equations:\n");
		print_equations(gems+len-1);
		printf("\n");
		printf("Amplifier equations:\n");
		print_equations_O(amps+len-1);
		printf("\n");
	}
	
	for (i=0;i<len;++i) free(pool[i]);			// free gems
	for (i=0;i<2*len;++i) free(poolO[i]);		// free amps
}


int main(int argc, char** argv)
{
	int len;
	int lenc;
	char opt;
	int output_parens=0;
	int output_equations=0;
	int output_tree=0;
	int output_table=0;
	int output_debug=0;
	int output_info=0;
	int sizes=2000;
	int sizec=100;

	while ((opt=getopt(argc,argv,"petcdis:"))!=-1) {
		switch(opt) {
			case 'p':
				output_parens = 1;
				break;
			case 'e':
				output_equations = 1;
				break;
			case 't':
				output_tree = 1;
				break;
			case 'c':
				output_table = 1;
				break;
			case 'd':
				output_debug = 1;
				output_info = 1;
				break;
			case 'i':
				output_info = 1;
				break;
			case 's':		// must be sizes,sizec
				;
				char* p=optarg;
				while (*p != ',') p++;
				*p='\0';
				sizes=atoi(optarg);
				sizec=atoi(p+1);
				break;
			case '?':
				return 1;
			default:
				break;
		}
	}
	if (optind+1==argc) {
		len = atoi(argv[optind]);
		lenc= len;
	}
	else if (optind+2==argc) {
		len = atoi(argv[optind]);
		lenc= atoi(argv[optind+1]);
	}
	else {
		printf("Unknown arguments:\n");
		while (argv[optind]!=NULL) {
			printf("%s ", argv[optind]);
			optind++;
		}
		return 1;
	}
	if (len<1 || lenc<1) printf("Improper gem number\n");
	else worker_omnia(len, lenc, output_parens, output_equations, output_tree, output_table, output_debug, output_info, sizes, sizec);
	return 0;
}