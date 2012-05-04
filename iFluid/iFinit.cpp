#include <iFluid.h>
#include <ifluid_basic.h>

static void kh_state(COMPONENT,double*,L_STATE&,int,IF_PARAMS*);
static void zero_state(COMPONENT,double*,L_STATE&,int,IF_PARAMS*);
static void initRayleiTaylorIntfc(Front*,LEVEL_FUNC_PACK*,char*);
static void initKHIntfc(Front*,LEVEL_FUNC_PACK*,char*);
static void initCirclePlaneIntfc(Front*,LEVEL_FUNC_PACK*,char*,IF_PROB_TYPE);
static void initChannelFlow(Front*,LEVEL_FUNC_PACK*,char*);

extern void setInitialIntfc(
	Front *front,
        LEVEL_FUNC_PACK *level_func_pack,
        char *inname,
	IF_PROB_TYPE prob_type)
{
        IF_PARAMS *iFparams = (IF_PARAMS*)front->extra1;
	iFparams->m_comp1 = LIQUID_COMP1;
	iFparams->m_comp2 = LIQUID_COMP2;
        switch (prob_type)
        {
	case TWO_FLUID_BUBBLE:
        case BUBBLE_SURFACE:
            initCirclePlaneIntfc(front,level_func_pack,inname,prob_type);
            break;
        case FLUID_SOLID_CIRCLE:
	    iFparams->m_comp1 = SOLID_COMP;
            initCirclePlaneIntfc(front,level_func_pack,inname,prob_type);
            break;
        case TWO_FLUID_RT:
            initRayleiTaylorIntfc(front,level_func_pack,inname);
            break;
        case TWO_FLUID_KH:
            initKHIntfc(front,level_func_pack,inname);
            break;
        case CHANNEL_FLOW:
            initChannelFlow(front,level_func_pack,inname);
            break;
	default:
	    (void) printf("In setInitialIntfc unknown type: %d\n",prob_type);
        }
}       /* end setInitialIntfc */

static void initRayleiTaylorIntfc(
	Front *front,
	LEVEL_FUNC_PACK *level_func_pack,
	char *inname)
{
	IF_PARAMS *iFparams = (IF_PARAMS*)front->extra1;
	static FOURIER_POLY level_func_params;
	static double L[MAXD],U[MAXD];
	FILE *infile = fopen(inname,"r");
	int i,j,dim,num_modes;
	char mesg[100];

	dim = level_func_params.dim = front->rect_grid->dim;
	level_func_params.L = L;
	level_func_params.U = U;
	for (i = 0; i < dim; ++i)
	{
	    level_func_params.L[i] = front->rect_grid->L[i];
	    level_func_params.U[i] = front->rect_grid->U[i];
	}

	level_func_pack->neg_component = LIQUID_COMP1;
        level_func_pack->pos_component = LIQUID_COMP2;
	level_func_pack->wave_type = FIRST_PHYSICS_WAVE_TYPE;
	CursorAfterString(infile,"Enter mean position of fluid interface:");
	fscanf(infile,"%lf",&level_func_params.z0);
	(void) printf("%f\n",level_func_params.z0);
	CursorAfterString(infile,"Enter number of sine modes:");
	fscanf(infile,"%d",&num_modes);
	(void) printf("%d",num_modes);
	level_func_params.num_modes = num_modes;
	FT_MatrixMemoryAlloc((POINTER*)&level_func_params.nu,num_modes,
			dim-1,sizeof(double));
	FT_VectorMemoryAlloc((POINTER*)&level_func_params.phase,num_modes,sizeof(double));
	FT_VectorMemoryAlloc((POINTER*)&level_func_params.A,num_modes,sizeof(double));
	for (i = 0; i < num_modes; ++i)
	{
	    sprintf(mesg,"Enter frequency of mode %d:",i+1);
	    CursorAfterString(infile,mesg);
	    for (j = 0; j < dim; ++j)
	    {
	    	fscanf(infile,"%lf",&level_func_params.nu[i][j]);
		(void) printf("%f ",level_func_params.nu[i][j]);
	    }
	    (void) printf("\n");
	    sprintf(mesg,"Enter amplitude of mode %d:",i+1);
	    CursorAfterString(infile,mesg);
	    fscanf(infile,"%lf",&level_func_params.A[i]);
	    (void) printf("%f\n",level_func_params.A[i]);
	    sprintf(mesg,"Enter phase of mode %d:",i+1);
	    CursorAfterString(infile,mesg);
	    fscanf(infile,"%lf",&level_func_params.phase[i]);
	    (void) printf("%f\n",level_func_params.phase[i]);
	}
	CursorAfterString(infile,"Enter density and viscosity of fluid 1:");
        fscanf(infile,"%lf %lf",&iFparams->rho1,&iFparams->mu1);
	(void) printf("%f %f\n",iFparams->rho1,iFparams->mu1);
        CursorAfterString(infile,"Enter density and viscosity of fluid 2:");
        fscanf(infile,"%lf %lf",&iFparams->rho2,&iFparams->mu2);
	(void) printf("%f %f\n",iFparams->rho2,iFparams->mu2);
	CursorAfterString(infile,"Enter gravity:");
        for (i = 0; i < dim; ++i)
	{
            fscanf(infile,"%lf",&iFparams->gravity[i]);
	    (void) printf("%f ",iFparams->gravity[i]);
	}
	(void) printf("\n");
	CursorAfterString(infile,"Enter surface tension:");
        fscanf(infile,"%lf",&iFparams->surf_tension);
	(void) printf("%f\n",iFparams->surf_tension);
	CursorAfterString(infile,"Enter factor of smoothing radius:");
        fscanf(infile,"%lf",&iFparams->smoothing_radius);
	(void) printf("%f\n",iFparams->smoothing_radius);

	level_func_pack->func_params = (POINTER)&level_func_params;
	level_func_pack->func = level_wave_func;
	fclose(infile);
}	/* end initRayleiTaylor */

static void initKHIntfc(
	Front *front,
	LEVEL_FUNC_PACK *level_func_pack,
	char *inname)
{
	IF_PARAMS *iFparams = (IF_PARAMS*)front->extra1;
	static FOURIER_POLY level_func_params;
	static double L[MAXD],U[MAXD];
	FILE *infile = fopen(inname,"r");
	int i,j,dim,num_modes;
	char mesg[100];

	dim = level_func_params.dim = front->rect_grid->dim;
	level_func_params.L = L;
	level_func_params.U = U;
	for (i = 0; i < dim; ++i)
	{
	    level_func_params.L[i] = front->rect_grid->L[i];
	    level_func_params.U[i] = front->rect_grid->U[i];
	}

	level_func_pack->neg_component = LIQUID_COMP1;
        level_func_pack->pos_component = LIQUID_COMP2;
	level_func_pack->wave_type = FIRST_PHYSICS_WAVE_TYPE;
	CursorAfterString(infile,"Enter mean position of fluid interface:");
	fscanf(infile,"%lf",&level_func_params.z0);
	CursorAfterString(infile,"Enter number of sine modes:");
	fscanf(infile,"%d",&num_modes);
	level_func_params.num_modes = num_modes;
	FT_MatrixMemoryAlloc((POINTER*)&level_func_params.nu,num_modes,
			dim-1,sizeof(double));
	FT_VectorMemoryAlloc((POINTER*)&level_func_params.phase,num_modes,sizeof(double));
	FT_VectorMemoryAlloc((POINTER*)&level_func_params.A,num_modes,sizeof(double));
	for (i = 0; i < num_modes; ++i)
	{
	    sprintf(mesg,"Enter frequency of mode %d:",i+1);
	    CursorAfterString(infile,mesg);
	    for (j = 0; j < dim; ++j)
	    	fscanf(infile,"%lf",&level_func_params.nu[i][j]);
	    sprintf(mesg,"Enter amplitude of mode %d:",i+1);
	    CursorAfterString(infile,mesg);
	    fscanf(infile,"%lf",&level_func_params.A[i]);
	    sprintf(mesg,"Enter phase of mode %d:",i+1);
	    CursorAfterString(infile,mesg);
	    fscanf(infile,"%lf",&level_func_params.phase[i]);
	}
	CursorAfterString(infile,"Enter density and viscosity of fluid 1:");
        fscanf(infile,"%lf %lf",&iFparams->rho1,&iFparams->mu1);
        CursorAfterString(infile,"Enter density and viscosity of fluid 2:");
        fscanf(infile,"%lf %lf",&iFparams->rho2,&iFparams->mu2);
        CursorAfterString(infile,"Enter bottom horizontal velocity:");
	for (j = 0; j < dim-1; ++j)
            fscanf(infile,"%lf",&iFparams->U1[j]);
        CursorAfterString(infile,"Enter top horizontal velocity:");
	for (j = 0; j < dim-1; ++j)
            fscanf(infile,"%lf",&iFparams->U2[j]);

	CursorAfterString(infile,"Enter gravity:");
        for (i = 0; i < dim; ++i)
            fscanf(infile,"%lf",&iFparams->gravity[i]);
	CursorAfterString(infile,"Enter surface tension:");
        fscanf(infile,"%lf",&iFparams->surf_tension);
	CursorAfterString(infile,"Enter factor of smoothing radius:");
        fscanf(infile,"%lf",&iFparams->smoothing_radius);

	level_func_pack->func_params = (POINTER)&level_func_params;
	level_func_pack->func = level_wave_func;
	fclose(infile);
}	/* end initRayleiTaylor */

static void initCirclePlaneIntfc(
	Front *front,
	LEVEL_FUNC_PACK *level_func_pack,
	char *inname,
	IF_PROB_TYPE prob_type)
{
        IF_PARAMS *iFparams = (IF_PARAMS*)front->extra1;
	FILE *infile = fopen(inname,"r");
	static CIRCLE_PARAMS *circle_params;
	int i,dim;

	iFparams = (IF_PARAMS*)front->extra1;
	FT_ScalarMemoryAlloc((POINTER*)&circle_params,sizeof(CIRCLE_PARAMS));
        circle_params->dim = dim = front->rect_grid->dim;
        circle_params->add_plan_surf = NO;
        CursorAfterString(infile,"Enter the center of the circle:");
        for (i = 0; i < dim; ++i)
	{
            fscanf(infile,"%lf",&circle_params->cen[i]);
            (void) printf("%f ",circle_params->cen[i]);
	}
	(void) printf("\n");
        CursorAfterString(infile,"Enter radius of the circle:");
        fscanf(infile,"%lf",&circle_params->R);
        (void) printf("%f\n",circle_params->R);

        if (prob_type == BUBBLE_SURFACE)
        {
            CursorAfterString(infile,"Enter height of the surface:");
            fscanf(infile,"%lf",&circle_params->H);
            (void) printf("%f\n",circle_params->H);
            circle_params->add_plan_surf = YES;
        }
	level_func_pack->func_params = (POINTER)circle_params;

	switch (prob_type)
	{
	case TWO_FLUID_BUBBLE:
        case BUBBLE_SURFACE:
            level_func_pack->neg_component = LIQUID_COMP1;
            level_func_pack->pos_component = LIQUID_COMP2;
            level_func_pack->func = level_circle_func;
            level_func_pack->wave_type = FIRST_PHYSICS_WAVE_TYPE;
	    CursorAfterString(infile,"Enter density and viscosity of fluid 1:");
            fscanf(infile,"%lf %lf",&iFparams->rho1,&iFparams->mu1);
            (void) printf("%f %f\n",iFparams->rho1,iFparams->mu1);
            CursorAfterString(infile,"Enter density and viscosity of fluid 2:");
            fscanf(infile,"%lf %lf",&iFparams->rho2,&iFparams->mu2);
            (void) printf("%f %f\n",iFparams->rho2,iFparams->mu2);
            break;
        case FLUID_SOLID_CIRCLE:
	    iFparams->m_comp1 = SOLID_COMP;
            level_func_pack->neg_component = SOLID_COMP;
            level_func_pack->pos_component = LIQUID_COMP2;
            level_func_pack->func = level_circle_func;
            level_func_pack->wave_type = NEUMANN_BOUNDARY;
            CursorAfterString(infile,
			"Enter density and viscosity of the fluid:");
            fscanf(infile,"%lf %lf",&iFparams->rho2,&iFparams->mu2);
            (void) printf("%f %f\n",iFparams->rho2,iFparams->mu2);
            break;
	default:
	    (void) printf("ERROR: entering wrong initialization function\n");
	    clean_up(ERROR);
	}
	CursorAfterString(infile,"Enter gravity:");
        for (i = 0; i < dim; ++i)
            fscanf(infile,"%lf\n",&iFparams->gravity[i]);
	CursorAfterString(infile,"Enter surface tension:");
        fscanf(infile,"%lf",&iFparams->surf_tension);
        (void) printf("%f\n",iFparams->surf_tension);
	CursorAfterString(infile,"Enter factor of smoothing radius:");
        fscanf(infile,"%lf",&iFparams->smoothing_radius);
        (void) printf("%f\n",iFparams->smoothing_radius);
	if (prob_type == TWO_FLUID_BUBBLE &&
	    iFparams->surf_tension != 0.0)	
	{
	    double mag_g = mag_vector(iFparams->gravity,dim);
	    double rho_b = iFparams->rho1;
	    double rho_o = iFparams->rho2;
	    double mu_b = iFparams->mu1;
	    double mu_o = iFparams->mu2;
	    double sigma = iFparams->surf_tension;
	    double de = 2.0*circle_params->R;
	    double V_morton = mag_g*sqr(mu_o)*sqr(mu_o)/rho_o/sqr(sigma)/sigma;
	    double V_eotvos = rho_o*mag_g*sqr(de)/sigma;
	    printf("Bubble simulation\n");
	    printf("Morton number   = %g\n",V_morton);
	    printf("Eotvos number   = %g\n",V_eotvos);
	    printf("Density ratio   = %g\n",rho_o/rho_b);
	    printf("Viscosity ratio = %g\n",mu_o/mu_b);
	}
	
	fclose(infile);	
}	/* end initCirclePlaneIntfc */

extern void init_fluid_state_func(
	Incompress_Solver_Smooth_Basis *l_cartesian,
	IF_PROB_TYPE prob_type)
{
	switch (prob_type)
	{
	case TWO_FLUID_KH:
	    l_cartesian->getInitialState = kh_state;
	    break;
	default:
	    l_cartesian->getInitialState = zero_state;
	}
}	/* end init_fluid_state_func */

static void kh_state(
	COMPONENT comp,
	double *coords,
	L_STATE& state,
	int dim,
	IF_PARAMS *iFparams)
{
	int i;
	for (i = 0; i < dim-1; ++i)
	{
	    state.m_U[i] = (comp == LIQUID_COMP1) ? iFparams->U1[i] :
				iFparams->U2[i];
	}
	state.m_U[dim-1] = 0.0;
}	/* end kh_state */

static void zero_state(
	COMPONENT comp,
	double *coords,
	L_STATE& state,
	int dim,
	IF_PARAMS *iFparams)
{
	int i;
	for (i = 0; i < dim; ++i)
	    state.m_U[i] = 0.0;
}	/* end zero_state */


extern void read_iF_prob_type(
	char *inname,
	IF_PROB_TYPE *prob_type)
{
	char string[100];
	FILE *infile = fopen(inname,"r");

	*prob_type = ERROR_TYPE;
	CursorAfterString(infile,"Enter problem type:");
	fscanf(infile,"%s",string);
	if (string[0] == 'T' || string[0] == 't')
	{
	    if (string[10] == 'B' || string[10] == 'b')
	    	*prob_type = TWO_FLUID_BUBBLE;
	    else if (string[10] == 'R' || string[10] == 'r')
	    	*prob_type = TWO_FLUID_RT;
	    else if (string[10] == 'K' || string[10] == 'k')
	    	*prob_type = TWO_FLUID_KH;
	} 
	else if (string[0] == 'F' || string[0] == 'f')
	{
            if (string[6] == 'S' || string[6] == 's')
                *prob_type = FLUID_SOLID_CIRCLE;
            else if (string[6] == 'R' || string[6] == 'r')
                *prob_type = FLUID_RIGID_BODY;
	}
	else if (string[0] == 'B' || string[0] == 'b')
	{
	    *prob_type = BUBBLE_SURFACE;
	}
        else if (string[0] == 'R' || string[0] == 'r')
	{
            if (string[6] == 'O' || string[6] == 'o')
                *prob_type = ROTOR_ONE_FLUID;
            else if (string[6] == 'T' || string[6] == 't')
                *prob_type = ROTOR_TWO_FLUID;
	}
	else if (string[0] == 'C' || string[0] == 'c')
	{
            *prob_type = CHANNEL_FLOW;
	}

	assert(*prob_type != ERROR_TYPE);
	fclose(infile);
}	/* end read_iFparams */

static void initChannelFlow(
	Front *front,
	LEVEL_FUNC_PACK *level_func_pack,
	char *inname)
{
        IF_PARAMS *iFparams = (IF_PARAMS*)front->extra1;
	FILE *infile = fopen(inname,"r");
	int i,dim = front->rect_grid->dim;

	iFparams = (IF_PARAMS*)front->extra1;

        level_func_pack->neg_component = LIQUID_COMP1;
        level_func_pack->pos_component = LIQUID_COMP2;
        level_func_pack->func = NULL;
        CursorAfterString(infile,"Enter density and viscosity of the fluid:");
        fscanf(infile,"%lf %lf",&iFparams->rho2,&iFparams->mu2);
        (void) printf("%f %f\n",iFparams->rho2,iFparams->mu2);
	CursorAfterString(infile,"Enter gravity:");
        for (i = 0; i < dim; ++i)
            fscanf(infile,"%lf",&iFparams->gravity[i]);
	
	fclose(infile);	
}	/* end initChannelFlow */

