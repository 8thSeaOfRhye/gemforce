#ifndef _CRIT_UTILS_H
#define _CRIT_UTILS_H

struct Gem_Y {
	int grade;				//using short does NOT improve time/memory usage
	float damage;
	float crit;
	struct Gem_Y* father;
	struct Gem_Y* mother;
};

void gem_print_Y(gemY *p_gem) {
	printf("Grade:\t%d\nDamage:\t%f\nCrit:\t%f\nPower:\t%f\n\n",
		p_gem->grade, p_gem->damage, p_gem->crit, p_gem->damage*p_gem->crit);
}

void gem_comb_eq_Y(gemY *p_gem1, gemY *p_gem2, gemY *p_gem_combined)
{
	p_gem_combined->grade = p_gem1->grade+1;
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = 0.87*p_gem1->damage + 0.71*p_gem2->damage;
	else p_gem_combined->damage = 0.87*p_gem2->damage + 0.71*p_gem1->damage;
	if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = 0.88*p_gem1->crit + 0.5*p_gem2->crit;
	else p_gem_combined->crit = 0.88*p_gem2->crit + 0.5*p_gem1->crit;
}

void gem_comb_d1_Y(gemY *p_gem1, gemY *p_gem2, gemY *p_gem_combined)     //bigger is always gem1
{
	p_gem_combined->grade = p_gem1->grade;
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = 0.86*p_gem1->damage + 0.7*p_gem2->damage;
	else p_gem_combined->damage = 0.86*p_gem2->damage + 0.7*p_gem1->damage;
	if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = 0.88*p_gem1->crit + 0.44*p_gem2->crit;
	else p_gem_combined->crit = 0.88*p_gem2->crit + 0.44*p_gem1->crit;
}

void gem_comb_gn_Y(gemY *p_gem1, gemY *p_gem2, gemY *p_gem_combined)
{
	p_gem_combined->grade = int_max(p_gem1->grade, p_gem2->grade);
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = 0.85*p_gem1->damage + 0.69*p_gem2->damage;
	else p_gem_combined->damage = 0.85*p_gem2->damage + 0.69*p_gem1->damage;
	if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = 0.88*p_gem1->crit + 0.44*p_gem2->crit;
	else p_gem_combined->crit = 0.88*p_gem2->crit + 0.44*p_gem1->crit;
}

void gem_combine_Y (gemY *p_gem1, gemY *p_gem2, gemY *p_gem_combined)
{
	p_gem_combined->father=p_gem1;
	p_gem_combined->mother=p_gem2;
	int delta = p_gem1->grade - p_gem2->grade;
	switch (delta){
		case 0:
			gem_comb_eq_Y(p_gem1, p_gem2, p_gem_combined);
			break;
		case 1:
			gem_comb_d1_Y(p_gem1, p_gem2, p_gem_combined);
			break;
		case -1:
			gem_comb_d1_Y(p_gem2, p_gem1, p_gem_combined);
			break;
		default:
			gem_comb_gn_Y(p_gem1, p_gem2, p_gem_combined);
			break;
	}
}

void gem_init_Y(gemY *p_gem, int grd, float damage, float crit)
{
	p_gem->grade =grd;
	p_gem->damage=damage;
	p_gem->crit=crit;
	p_gem->father=NULL;
	p_gem->mother=NULL;
}

int gem_has_less_damage_crit(gemY gem1, gemY gem2)
{
	if (gem1.damage < gem2.damage) return 1;
	else if (gem1.damage == gem2.damage && gem1.crit < gem2.crit) return 1;
	else return 0;
}

void gem_sort_Y (gemY* gems, int len) {
	if (len < 2) return;
	gemY pivot = gems[len/2];
	gemY* beg = gems;
	gemY* end = gems+len-1;
	while (beg <= end) {
		if (gem_has_less_damage_crit(*beg, pivot)) {
			beg++;
		}
		else if (gem_has_less_damage_crit(pivot,*end)) {
			end--;
		}
		else {
			gemY temp = *beg;
			*beg = *end;
			*end = temp;
			beg++;
			end--;
		}
	}
	gem_sort_Y(gems, end-gems+1);
	gem_sort_Y(beg, gems-beg+len);
}

void print_parens_Y(gemY* gemf)
{
	if (gemf->father==NULL) printf("y");
	else {
		printf("(");
		print_parens_Y(gemf->father);
		printf("+");
		print_parens_Y(gemf->mother);
		printf(")");
	}
	return;
}

int gem_getvalue_Y(gemY* p_gem)
{
	if (p_gem->grade==0) return 0;
	if (p_gem->father==NULL) return 1;
	else return gem_getvalue_Y(p_gem->father)+gem_getvalue_Y(p_gem->mother);
}

void fill_array_Y(gemY* gemf, gemY** p_gems, int* place)
{
	if (gemf-> father != NULL) {
		fill_array_Y(gemf->father, p_gems, place);
		fill_array_Y(gemf->mother, p_gems, place);
	}
	int i;
	int uniq=1;
	for (i=0; i<*place; ++i) if (gemf==p_gems[i]) uniq=0;
	if (uniq) {
		gemf->grade+=1000*(*place);			// mark
		p_gems[*place]=gemf;
		(*place)++;
	}
}

void print_eq_Y(gemY* p_gem, int* printed_uid)
{
	if (printed_uid[p_gem->grade/1000]==1) return;
	if (gem_getvalue_Y(p_gem)==1) printf("(val = 1)\t%2d = g1 y\n", p_gem->grade/1000);
	else {
		print_eq_Y(p_gem->father, printed_uid);
		print_eq_Y(p_gem->mother, printed_uid);
		if (gem_getvalue_Y(p_gem->father) > gem_getvalue_Y(p_gem->father)) {
			printf("(val = %d)\t%2d = %2d + %2d\n", gem_getvalue_Y(p_gem), p_gem->grade/1000, p_gem->father->grade/1000, p_gem->mother->grade/1000);
		}
		else {
			printf("(val = %d)\t%2d = %2d + %2d\n", gem_getvalue_Y(p_gem), p_gem->grade/1000, p_gem->mother->grade/1000, p_gem->father->grade/1000);
		}
	}
	printed_uid[p_gem->grade/1000]=1;
}

void print_equations_Y(gemY* gemf)
{
	int value=gem_getvalue_Y(gemf);
	int len=2*value-1;
	gemY** p_gems=malloc(len*sizeof(gemY*));		// let's store all the gem pointers
	int place=0;
	fill_array_Y(gemf, p_gems, &place);					// this array contains marked uniques only and is long "place"
	int i;
	int printed_uid[place];
	for (i=0; i<place; ++i) printed_uid[i]=0;
	print_eq_Y(gemf, printed_uid);
	free(p_gems);
}

void print_tree_Y(gemY* gemf, char* prefix)
{
	if (gemf->father==NULL) {
		printf("━ g1 y\n");
	}
	else {
		printf("━%d\n",gem_getvalue_Y(gemf));
		printf("%s ┣",prefix);
		char string[strlen(prefix)+2];
		strcpy(string,prefix);
		strcat(string," ┃");
		gemY* gem1;
		gemY* gem2;
		if (gem_getvalue_Y(gemf->father)>gem_getvalue_Y(gemf->mother)) {
			gem1=gemf->father;
			gem2=gemf->mother;
		}
		else {
			gem2=gemf->father;
			gem1=gemf->mother;
		}
		print_tree_Y(gem1, string);
		printf("%s ┗",prefix);
		char string2[strlen(prefix)+2];
		strcpy(string2,prefix);
		strcat(string2,"  ");
		print_tree_Y(gem2, string2);
	}
}

#endif // _CRIT_UTILS_H
