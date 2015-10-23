#include "swap.h"

static void SlopeBasedTrade(DATA_SET*);
static void Slope1BasedTrade(DATA_SET*);
static void IntegralSlopeTrade(DATA_SET*);
static void PyramidTrade(DATA_SET*);
static void ComputePyramidParams(DATA_SET*,double*,double*);
static void PrintDevLinearProfile(char**,double*,double*,double,double,
				int*,int);

extern void InvestSimulation(
	DATA_SET *data)
{
	int i_method;
	DATA_SET *copy_data = CopyData(data);
	printf("Available trading methods are:\n");
	printf("\tSlope-based trading (0)\n");
	printf("\tLeast-Square-Slope-based trading (1)\n");
	printf("\tIntegral-Slope trading (2)\n");
	printf("\tPyramid trading (3)\n");
	printf("Enter trading method: ");
	scanf("%d",&i_method);
	switch (i_method)
	{
	case 0:
	    SlopeBasedTrade(copy_data);
	    break;
	case 1:
	    Slope1BasedTrade(copy_data);
	    break;
	case 2:
            IntegralSlopeTrade(copy_data);
	    break;
	case 3:
	    PyramidTrade(copy_data);
	    break;
	default:
	    printf("Unknown trading method\n");
	    return;
	}
	FreeData(copy_data);
}	/* end InvestSimulation */

static void SlopeBasedTrade(
	DATA_SET *data)
{
	int i,j,i1,i2,is,N;
	int M = data->num_assets-1;
	double LA,LB;
	double QA,QB,QC;
	double S0[5],S[5];
	double s0[5],s1[5],q1[5];
	double s0_max,s1_max;
	double *value;
	double time[MAX_TRACE];
	int num_swap = 0;
	int is1_max,is0_max;
	double total_value0,total_value;
	char string[100];
	double dt;
	int ns1;	/* number of point for secondary slope */
	boolean cash_swap = NO;
	double cash;

start_trade:

	N = data->num_backtrace - 5;
	if (N > MAX_TRACE - 5) N = MAX_TRACE - 5;
	printf("Enter time interval: ");
	scanf("%lf",&dt);
	printf("Enter number of point for secondary slope: ");
	scanf("%d",&ns1);
	printf("Enter yes to include cash swap: ");
	scanf("%s",string);
	cash_swap = NO;
	cash = 0.0;
	num_swap = 0;
	if (string[0] == 'y' || string[0] == 'Y')
	{
	    cash_swap = YES;
	}

	total_value0 = 0.0;
	is = data->num_days - N;
	for (i = 0; i < M; ++i)
	{
	    printf("For stock %s, enter starting number of shares: ",
				data->assets[i].asset_name);
	    scanf("%lf",S0+i);
	    total_value0 += S0[i]*data->assets[i].value[is];
	}
	printf("\nTotal starting value = %f\n",total_value0);
	total_value = 0.0;
	is = data->num_days - 1;
	for (i = 0; i < M; ++i)
	{
	    total_value += S0[i]*data->assets[i].value[is];
	}
	printf("Total number of steps = %d\n",N);
	printf("Total untraded end value = %f\n",total_value);
	printf("Change: %f percent\n",100*(total_value/total_value0 - 1.0));
	printf("\n");

	for (i = 0; i < N; ++i) 
	    time[i] = i*dt;

	for (j = 0; j < N; ++j)
	{
	    is = data->num_days - N + j - (ns1 - 1);
	    s0_max = s1_max = -HUGE;
	    for (i = 0; i < M; ++i)
	    {
	    	value = data->assets[i].norm_value + is;
		s0[i] = value[ns1-1] - value[ns1-2];
		S[i] = data->assets[i].value[is+(ns1-1)];
		LeastSquareLinear(time,value,ns1,&LA,&LB);
		LeastSquareQuadr(time,value,ns1,&QA,&QB,&QC);
		s1[i] = LA;
		q1[i] = QA;
		if (s0_max < s0[i]) 
		{
		    s0_max = s0[i];
		    is0_max = i;
		}
		if (s1_max < s1[i]) 
		{
		    s1_max = s1[i];
		    is1_max = i;
		}
	    }
	    for (i1 = 0; i1 < M; ++i1)
	    {
		if (cash_swap)
		{
		    if (S0[i1] != 0.0 && s0[is0_max] < 0.0 &&
			s1[i1] < 0.0)
		    {
		    	cash += S0[i1]*S[i1];
		    	S0[i1] = 0.0;
		    	num_swap++;
		    }
		    if (cash != 0.0 && s0[is0_max] > 0.0 && 
			i1 == is0_max && s1[i1] > 0.0)
		    {
			S0[i1] += cash/S[i1];
			cash = 0.0;
		    	num_swap++;
		    }
		}
		if (
		    S0[i1] != 0.0 
		    && i1 != is0_max
		    && s1[i1] < s1[is0_max]
		   )
		{
		    S0[is0_max] += S0[i1]*S[i1]/S[is0_max];
		    S0[i1] = 0.0;
		    num_swap++;
		}
	    }
	}
	total_value = 0.0;
	is = data->num_days - 1;
	for (i = 0; i < M; ++i)
	{
	    printf("For stock %s, end number of shares: %f\n",
				data->assets[i].asset_name,S0[i]);
	    total_value += S0[i]*data->assets[i].value[is];
	}
	total_value += cash;
	printf("Total traded end value = %f\n",total_value);
	printf("Change: %f percent\n",100*(total_value/total_value0 - 1.0));
	printf("Number of swaps = %d\n",num_swap);
	printf("Type yes to restart simulation: ");
	scanf("%s",string);
	if (string[0] == 'y' || string[0] == 'Y')
	    goto start_trade;
}	/* end SlopeBasedTrade */

static void Slope1BasedTrade(
	DATA_SET *data)
{
	int i,j,i1,i2,is,N;
	int M = data->num_assets-1;
	double LA,LB;
	double QA,QB,QC;
	double S0[5],S[5];
	double s0[5],s1[5],q1[5];
	double s0_max,s1_max;
	double *value;
	double time[MAX_TRACE];
	int num_swap = 0;
	int is1_max,is0_max;
	double total_value0,total_value;
	char string[100];
	double dt;
	int ns1;	/* number of point for secondary slope */
	boolean cash_swap = NO;
	double cash;

start_trade:

	N = data->num_backtrace - 5;
	if (N > MAX_TRACE - 5) N = MAX_TRACE - 5;
	printf("Enter time interval: ");
	scanf("%lf",&dt);
	printf("Enter number of point for secondary slope: ");
	scanf("%d",&ns1);
	printf("Enter yes to include cash swap: ");
	scanf("%s",string);
	cash_swap = NO;
	cash = 0.0;
	num_swap = 0;
	if (string[0] == 'y' || string[0] == 'Y')
	{
	    cash_swap = YES;
	}

	total_value0 = 0.0;
	is = data->num_days - N;
	for (i = 0; i < M; ++i)
	{
	    printf("For stock %s, enter starting number of shares: ",
				data->assets[i].asset_name);
	    scanf("%lf",S0+i);
	    total_value0 += S0[i]*data->assets[i].value[is];
	}
	printf("\nTotal starting value = %f\n",total_value0);
	total_value = 0.0;
	is = data->num_days - 1;
	for (i = 0; i < M; ++i)
	{
	    total_value += S0[i]*data->assets[i].value[is];
	}
	printf("Total untraded end value = %f\n",total_value);
	printf("Change: %f percent\n",100*(total_value/total_value0 - 1.0));
	printf("\n");

	for (i = 0; i < N; ++i) 
	    time[i] = i*dt;

	for (j = 0; j < N; ++j)
	{
	    is = data->num_days - N + j - (ns1 - 1);
	    s0_max = s1_max = -HUGE;
	    for (i = 0; i < M; ++i)
	    {
	    	value = data->assets[i].norm_value + is;
		s0[i] = value[ns1-1] - value[ns1-2];
		S[i] = data->assets[i].value[is+(ns1-1)];
		LeastSquareLinear(time,value,ns1,&LA,&LB);
		LeastSquareQuadr(time,value,ns1,&QA,&QB,&QC);
		s1[i] = LA;
		q1[i] = QA;
		if (s0_max < s0[i]) 
		{
		    s0_max = s0[i];
		    is0_max = i;
		}
		if (s1_max < s1[i]) 
		{
		    s1_max = s1[i];
		    is1_max = i;
		}
	    }
	    for (i1 = 0; i1 < M; ++i1)
	    {
		if (cash_swap)
		{
		    if (S0[i1] != 0.0 && s0[is0_max] < 0.0 &&
			s1[i1] < 0.0)
		    {
		    	cash += S0[i1]*S[i1];
		    	S0[i1] = 0.0;
		    	num_swap++;
		    }
		    if (cash != 0.0 && s0[is0_max] > 0.0 && 
			i1 == is0_max && s1[i1] > 0.0)
		    {
			S0[i1] += cash/S[i1];
			cash = 0.0;
		    	num_swap++;
		    }
		}
		if (
		    S0[i1] != 0.0 
		    && i1 != is1_max
		    //&& s1[i1] < s1[is0_max]
		   )
		{
		    S0[is1_max] += S0[i1]*S[i1]/S[is1_max];
		    S0[i1] = 0.0;
		    num_swap++;
		}
	    }
	}
	total_value = 0.0;
	is = data->num_days - 1;
	for (i = 0; i < M; ++i)
	{
	    printf("For stock %s, end number of shares: %f\n",
				data->assets[i].asset_name,S0[i]);
	    total_value += S0[i]*data->assets[i].value[is];
	}
	total_value += cash;
	printf("Total traded end value = %f\n",total_value);
	printf("Change: %f percent\n",100*(total_value/total_value0 - 1.0));
	printf("Number of swaps = %d\n",num_swap);
	printf("Type yes to restart simulation: ");
	scanf("%s",string);
	if (string[0] == 'y' || string[0] == 'Y')
	    goto start_trade;

} 	/* end Slope1BasedTrade */

#define		MAX_SIM_NUM		100
static void IntegralSlopeTrade(
	DATA_SET *data)
{
	int i,j,i1,i2,is,N;
	int M = data->num_assets;
	double LA,LB;
	double S0[MAX_SIM_NUM],S[MAX_SIM_NUM],nS[MAX_SIM_NUM];
	double s0[MAX_SIM_NUM],s1[MAX_SIM_NUM];
	double *value;
	double time[MAX_TRACE];
	int num_swap = 0;
	double total_value0,total_value;
	char string[100];
	double dt = 1.0;
	int ns1;	/* number of point for secondary slope */
	double dv;
	int itmp,isort[200];
	int count = 0;
	FILE *tfile;
	char tfile_name[100];

	printf("Type yes to adjust base: ");
	scanf("%s",string);
	if (string[0] == 'y' || string[0] == 'Y')
	    ContinueBaseAdjust(data);
start_trade:

	create_directory("trade",NO);
	sprintf(tfile_name,"trade/record-%d",count);
	tfile = fopen(tfile_name,"w");
	N = data->num_backtrace - 5;
	if (N > MAX_TRACE - 5) N = MAX_TRACE - 5;
	printf("Enter number of point for slope: ");
	scanf("%d",&ns1);
	printf("Enter normalized integral difference for trading: ");
	scanf("%lf",&dv);
	num_swap = 0;

	total_value0 = 0.0;
	is = data->num_days - N;
	for (i = 0; i < M; ++i)
	{
	    printf("For stock %s, enter starting number of shares: ",
				data->assets[i].asset_name);
	    scanf("%lf",S0+i);
	    total_value0 += S0[i]*data->assets[i].value[is];
	}
	printf("\nTotal starting value = %f\n",total_value0);
	fprintf(tfile,"\nTotal starting value = %f\n",total_value0);
	total_value = 0.0;
	is = data->num_days - 1;
	fprintf(tfile,"Starting assets:\n");
	for (i = 0; i < M; ++i)
	{
	    total_value += S0[i]*data->assets[i].value[is];
	    fprintf(tfile,"%5s   %f    %10.2f\n",
				data->assets[i].asset_name,
				S0[i],data->assets[i].value[is]);
	}
	printf("Total untraded end value = %f\n",total_value);
	printf("Change: %f percent\n",100*(total_value/total_value0 - 1.0));
	printf("\n");
	fprintf(tfile,"Total untraded end value = %f\n",total_value);
	fprintf(tfile,"Change: %f percent\n",
				100*(total_value/total_value0 - 1.0));
	fprintf(tfile,"\n");

	for (i = 0; i < N; ++i) 
	    time[i] = i*dt;

	for (j = 0; j < N; ++j)
	{
	    is = data->num_days - N + j - (ns1 - 1);
	    for (i = 0; i < M; ++i)
	    {
	    	value = data->assets[i].norm_value + is;
		s0[i] = value[ns1-1] - value[ns1-2];
		S[i] = data->assets[i].value[is+(ns1-1)];
		nS[i] = data->assets[i].norm_value[is+(ns1-1)];
		LeastSquareLinear(time,value,ns1,&LA,&LB);
		s1[i] = LA;
		isort[i] = i;
	    }
	    for (i1 = 0; i1 < M; ++i1)
	    for (i2 = 0; i2 < M; ++i2)
	    {
		if (nS[isort[i1]] < nS[isort[i2]])
	 	{
		    itmp = isort[i1];
		    isort[i1] = isort[i2];
		    isort[i2] = itmp;
		}
	    }
	    for (i1 = 0; i1 < M; ++i1)
	    for (i2 = 0; i2 < M; ++i2)
	    {
		if (i1 == i2) continue;
		if (
		    S0[i1] != 0.0 
		    && s1[i1] > s1[i2]
		    && 100.0*(nS[i1] - nS[i2]) > dv
		   )
		{
		    S0[i2] += S0[i1]*S[i1]/S[i2];
		    S0[i1] = 0.0;
		    num_swap++;
		}
	    }
	}
	total_value = 0.0;
	is = data->num_days - 1;
	fprintf(tfile,"Ending assets:\n");
	for (i = 0; i < M; ++i)
	{
	    printf("For stock %s, end number of shares: %f\n",
				data->assets[i].asset_name,S0[i]);
	    total_value += S0[i]*data->assets[i].value[is];
	    fprintf(tfile,"%5s   %f    %10.2f\n",
				data->assets[i].asset_name,
				S0[i],data->assets[i].value[is]);
	}
	fprintf(tfile,"Total traded end value = %f\n",total_value);
	fprintf(tfile,"Change: %f percent\n",
			100*(total_value/total_value0 - 1.0));
	fprintf(tfile,"Number of swaps = %d\n",num_swap);
	fclose(tfile);
	printf("Total traded end value = %f\n",total_value);
	printf("Change: %f percent\n",100*(total_value/total_value0 - 1.0));
	printf("Number of swaps = %d\n",num_swap);
	printf("Type yes to restart simulation: ");
	scanf("%s",string);
	if (string[0] == 'y' || string[0] == 'Y')
	{
	    count++;
	    goto start_trade;
	}
}	/* end IntegralSlopeTrade */

static void PyramidTrade(
	DATA_SET *data)
{
	int i, j, k, i1, i2;
	int is, isf, N;
	int M = data->num_assets-1;
	
	double des_S0[100], ini_cash[100], old_value[100], new_invest[100];  
	double tmp_os[100], tmp_nS[100], percent[100];
	double linear_value[100];
	double *norm_price[100];
	
	char string[100];
	double total_value0, total_value;
	
	int itmp,isort[200];
	int num_swap = 0;
	double slope,b;		// Slope and intercept of linear profile

	printf("Type yes to adjust base: ");
	scanf("%s", string);
	if (string[0] == 'Y' || string[0] == 'y')
	   ContinueBaseAdjust(data);

start_trade:
	printf("Enter slope of the profile: ");
	scanf("%lf",&slope);
	N = data->num_backtrace;
	if (N > MAX_TRACE) N = MAX_TRACE;
	is = data->num_days - N;  /* starting date */
	isf = data->num_days - 1; /* ending date */

	for (i = 0; i <= M; ++i)
	{
	    isort[i] = i;
	    tmp_os[i] = data->assets[i].value[is];
	    tmp_nS[i] =  data->assets[i].norm_value[is];
	    norm_price[i] = data->assets[i].norm_value;
	}
	for (i1 = 0; i1 < M; ++i1)
	{
	    for (i2 = i1 + 1; i2 <= M; ++i2)
	    {
	        if (tmp_nS[isort[i1]] < tmp_nS[isort[i2]]) 
	        {
		    itmp = isort[i1];
		    isort[i1] = isort[i2];
		    isort[i2] = itmp;
		} 
	    }
	}

	printf("Enter total investment value: ");
	scanf("%lf",&total_value0);
	/* Get each stock num according to investment value */
	for (i = 0; i <= M; ++i)
	{
	    des_S0[isort[i]] = total_value0/tmp_os[isort[i]]/(M+1);  
	}

	/* Calculate the intercept */
	b = total_value0;
	for (i = 0; i <= M; ++i)
	{
	    b -= slope*data->assets[isort[i]].norm_value[is];
	    percent[i] = ini_cash[i]/total_value0; 
	}
	b /= (M + 1);

	printf("\nTotal starting value = %f\n",total_value0);	
	printf("slope = %f  intercept = %f\n",slope,b);
	total_value = 0.0;
	for (i = 0; i <= M; ++i) 
	{
	    total_value += des_S0[isort[i]]*data->assets[isort[i]].value[isf];
	    linear_value[i] = slope*data->assets[i].norm_value[is] + b;
	}
	printf("Sorted starting prices:\n");
	for (i = 0; i <= M; ++i)
	{
	    printf("%4s: %f  %8.2f  %8f\n",data->assets[isort[i]].asset_name,
				data->assets[isort[i]].norm_value[is],
				linear_value[isort[i]],
				des_S0[isort[i]]*tmp_os[isort[i]]);
	}

	printf("Total untraded end value = %f\n",total_value);
	printf("Change: %f percent\n",100 * (total_value/total_value0 - 1.0));
	printf("\n");
	exit(0);


	for (i = is + 1; i <=  isf; i++) 
	{			
	    total_value = 0;    
	    for (j = 0; j <= M; j++)    
	    {
		tmp_nS[isort[j]] = data->assets[isort[j]].norm_value[i];
		tmp_os[isort[j]] = data->assets[isort[j]].value[i];
		old_value[j] = tmp_os[isort[j]] * des_S0[isort[j]];
		total_value += old_value[j];
	    }
	    /* order change due to Norm value */
	    for (j = 0; j < M; j++ )   
	    {
		for (k = j + 1; k <= M; k++)
		{
		    if (tmp_nS[isort[j]] <  tmp_nS[isort[k]])
		    {
		       itmp = isort[j];
		       isort[j] = isort[k];
		       isort[k] = itmp;
		    }
		}
	    }
	    /* each stock number des_S0[i] updates due to triangle holding */
	    for (j = 0; j <= M; j++) 
	    {
		new_invest[j] =  total_value * percent[j];
		if (new_invest[j] > old_value[j])
		{
		   des_S0[isort[j]] += (new_invest[j] - old_value[j])
				/tmp_os[isort[j]];
		   num_swap ++;
		} else if(new_invest[j] < old_value[j])
		{
		   des_S0[isort[j]] -= (old_value[j] - new_invest[j])
				/tmp_os[isort[j]]; 
		   num_swap ++;
		}
	    }
	}   /* end first for */
	total_value = 0;
	/* Get total value in the last day */
	for (i = 0; i <= M ; i++)
	{
	    total_value += des_S0[i] * data->assets[i].value[isf];  
	}
	printf("Total traded end value = %f\n",total_value);
	printf("Change: %f percent\n",100*(total_value/total_value0 - 1.0));
	printf("Number of swaps = %d\n",num_swap);
	printf("Type yes to restart simulation: ");
	scanf("%s",string);
	if (string[0] == 'y' || string[0] == 'Y')
	    goto start_trade;
} 	/* end	PyramidTrade */

extern void PrintCurrentLinearProfile(DATA_SET *data)
{
	double *price,*nprice;
	int *num_shares;
	char **names;
	double slope,b;
	int i,id,n,M,N;
	N = data->num_assets;
	id = data->num_days - 1;

	FT_VectorMemoryAlloc((POINTER*)&price,N,sizeof(double));
	FT_VectorMemoryAlloc((POINTER*)&nprice,N,sizeof(double));
	FT_VectorMemoryAlloc((POINTER*)&num_shares,N,sizeof(int));
	FT_VectorMemoryAlloc((POINTER*)&names,N,sizeof(char*));
	n = 0;
	for (i = 0; i < N; ++i)
	{
	    if (!data->data_map[i])
		continue;
	    price[n] = data->assets[i].value[id];
	    nprice[n] = data->assets[i].norm_value[id];
	    num_shares[n] = data->shares[i];
	    names[n] = data->assets[i].asset_name;
	    n++;
	}
	M = n;
	ComputePyramidParams(data,&slope,&b);
	PrintDevLinearProfile(names,price,nprice,slope,b,num_shares,M);
	FT_FreeThese(3,price,nprice,num_shares);
}	/* end PrintCurrentLinearProfile */

static void PrintDevLinearProfile(
	char **names,
	double *price,
	double *norm_price,
	double slope,		// Slope for A_i = slope*p_i + b
	double b,		// Intercept for A_i = slope*p_i + b
	int *num_shares,
	int M)
{
	int i,j,i1,i2;
	int itmp,isort[100];
	double linear_value[100];
	double current_value[100];
	double market_value = 0.0;
	int lin_num_shares[100];

	for (i = 0; i < M; ++i)
	    isort[i] = i;
	for (i1 = 0; i1 < M-1; ++i1)
	{
	    for (i2 = i1 + 1; i2 < M; ++i2)
	    {
	        if (norm_price[isort[i1]] < norm_price[isort[i2]]) 
	        {
		    itmp = isort[i1];
		    isort[i1] = isort[i2];
		    isort[i2] = itmp;
		} 
	    }
	}
	for (i = 0; i < M; ++i)
	    market_value += price[i]*num_shares[i];

	market_value = 0.0;
	for (i = 0; i < M; ++i)
	{
	    linear_value[i] = slope*norm_price[i] + b;
	    lin_num_shares[i] = linear_value[i]/price[i];
	    current_value[i] = num_shares[i]*price[i];
	}
	printf("Equity NPrice   L-Val    C-Val    D-Val  "
		"L-Share  C-Share  D-Share\n");
	for (i = 0; i < M; ++i)
	{
	    i1 = isort[i];
	    printf("  %4s  %5.2f  %6.0f   %6.0f   %6.0f   %6d   %6d   %6d\n",
				names[i1],norm_price[i1],
				linear_value[i1],current_value[i1],
				current_value[i1]-linear_value[i1],
				lin_num_shares[i1],num_shares[i1],
				num_shares[i1]-lin_num_shares[i1]);
			
	}
}	/* end PrintDevLinearProfile */

static void ComputePyramidParams(
	DATA_SET *data,
	double *slope,
	double *intercept)
{
	double a[2],b[2],c[2],x[2];
	int i,N,M = data->num_assets;
	int n = data->num_days - 1;
	int m = 4;
	double pmin,pmax;
	int imin,imax;

	a[0] = 0.0;
	c[0] = c[1] = 0.0;
	pmin = HUGE;
	pmax = -HUGE;
	N = 0;
	for (i = 0; i < M; ++i)
	{
	    if (!data->data_map[i]) continue;
	    if (data->assets[i].norm_value[n] < pmin)
	    {
		pmin = data->assets[i].norm_value[n];
		imin = i;
	    }
	    if (data->assets[i].norm_value[n] > pmax)
	    {
		pmax = data->assets[i].norm_value[n];
		imax = i;
	    }
	    a[0] += data->assets[i].norm_value[n];
	    c[0] += data->assets[i].value[n]*data->shares[i];
	    N++;
	}
	a[1] = data->assets[imin].norm_value[n] - 
			m*data->assets[imax].norm_value[n];
	b[0] = (double)N;	
	b[1] = (1.0 - m);
	linear_22_equation(a,b,c,x);
	*slope = x[0];
	*intercept = x[1];
}	/* end ComputePyramidParams */
