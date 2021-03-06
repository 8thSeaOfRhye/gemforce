#ifndef _PRINT_UTILS_H
#define _PRINT_UTILS_H

int gem_getvalue(gem* p_gem)
{
	if(p_gem->father==NULL) return 1;
	else return gem_getvalue(p_gem->father)+gem_getvalue(p_gem->mother);
}

void print_parens(gem* gemf)
{
	if (gemf->father==NULL) printf("%c",gem_color(gemf));
	else {
		printf("(");
		print_parens(gemf->father);
		printf("+");
		print_parens(gemf->mother);
		printf(")");
	}
	return;
}

void fill_array(gem* gemf, gem** p_gems, int* place)
{
	if (gemf-> father != NULL) {
		fill_array(gemf->father, p_gems, place);
		fill_array(gemf->mother, p_gems, place);
	}
	int i;
	int uniq=1;
	for (i=0; i<*place; ++i) if (gemf==p_gems[i]) uniq=0;
	if (uniq) {
		gemf->grade=*place;			// mark
		p_gems[*place]=gemf;
		(*place)++;
	}
}

void print_eq(gem* p_gem, int* printed_uid)
{
	if (printed_uid[p_gem->grade]==1) return;
	if (gem_getvalue(p_gem)==1) printf("(val = 1)\t%2d = g1 %c\n", p_gem->grade, gem_color(p_gem));
	else {
		print_eq(p_gem->father, printed_uid);
		print_eq(p_gem->mother, printed_uid);
		if (gem_getvalue(p_gem->father) > gem_getvalue(p_gem->father)) {
			printf("(val = %d)\t%2d = %2d + %2d\n", gem_getvalue(p_gem), p_gem->grade, p_gem->father->grade, p_gem->mother->grade);
		}
		else {
			printf("(val = %d)\t%2d = %2d + %2d\n", gem_getvalue(p_gem), p_gem->grade, p_gem->mother->grade, p_gem->father->grade);
		}
	}
	printed_uid[p_gem->grade]=1;
}

void print_equations(gem* gemf)
{
	int value=gem_getvalue(gemf);
	int len=2*value-1;
	gem** p_gems=malloc(len*sizeof(gem*));		// let's store all the gem pointers
	int place=0;
	fill_array(gemf, p_gems, &place);					// this array contains marked uniques only and is long "place"
	int i;
	int printed_uid[place];
	for (i=0; i<place; ++i) printed_uid[i]=0;
	print_eq(gemf, printed_uid);
	free(p_gems);
}

void print_tree(gem* gemf, char* prefix)
{
	if (gemf->father==NULL) {
		printf("━ g1 %c\n",gem_color(gemf));
	}
	else {
		printf("━%d\n",gem_getvalue(gemf));
		printf("%s ┣",prefix);
		char string[strlen(prefix)+2];
		strcpy(string,prefix);
		strcat(string," ┃");
		gem* gem1;
		gem* gem2;
		if (gem_getvalue(gemf->father)>gem_getvalue(gemf->mother)) {
			gem1=gemf->father;
			gem2=gemf->mother;
		}
		else {
			gem2=gemf->father;
			gem1=gemf->mother;
		}
		print_tree(gem1, string);
		printf("%s ┗",prefix);
		char string2[strlen(prefix)+2];
		strcpy(string2,prefix);
		strcat(string2,"  ");
		print_tree(gem2, string2);
	}
}

void print_table(gem* gems, int len)
{
	printf("# Gems\tPower\n");
	int i;
	for (i=0;i<len;i++) printf("%d\t%.6lf\n",i+1,gem_power(gems[i]));
	printf("\n");
}

#endif // _PRINT_UTILS_H
