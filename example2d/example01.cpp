/***************************************************************
FronTier is a set of libraries that implements different types of 
Front Traking algorithms. Front Tracking is a numerical method for 
the solution of partial differential equations whose solutions have 
discontinuities.  

Copyright (C) 1999 by The University at Stony Brook. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
****************************************************************/

/*
*				example01.c:
*
*		User initialization example for Front Package:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*	
*	This example shows interface evolution of a circle in the double 
*       vortex field. It demonstrates the subgrid resolution of the front 
*       tracking method.
*
*/

#include <vector>
#include <FronTier.h>

	/*  Function Declarations */
static void test_propagate(Front*);

char *in_name,*restart_state_name,*restart_name,*out_name;
boolean RestartRun;
int RestartStep;

/********************************************************************
 *	Velocity function parameters for the front	 	    *
 ********************************************************************/

typedef struct {
	double i1,i2;
        double cen1[2],cen2[2];
} DOUBLE_VORTEX_PARAMS;

int main(int argc, char **argv)
{
	static Front front;
	static RECT_GRID comp_grid;
	static F_BASIC_DATA f_basic;
	static LEVEL_FUNC_PACK level_func_pack;
	static VELO_FUNC_PACK velo_func_pack;
	static CIRCLE_PARAMS circle_params;	/* level function parameters */
	static BIPOLAR_PARAMS dv_params; /* velocity function parameters */
	Locstate  sl;

	FT_Init(argc,argv,&f_basic);
	f_basic.dim = 2;

	/* Initialize basic computational data */

	f_basic.L[0] = 0.0;	f_basic.L[1] = 0.0;
	f_basic.U[0] = 1.0;	f_basic.U[1] = 1.0;
	f_basic.gmax[0] = 100;	f_basic.gmax[1] = 100;
	f_basic.boundary[0][0] = f_basic.boundary[0][1] = PERIODIC_BOUNDARY;
	f_basic.boundary[1][0] = f_basic.boundary[1][1] = PERIODIC_BOUNDARY;
        f_basic.size_of_intfc_state = 0;

        in_name                 = f_basic.in_name;
        restart_state_name      = f_basic.restart_state_name;
        out_name                = f_basic.out_name;
        restart_name            = f_basic.restart_name;
        RestartRun              = f_basic.RestartRun;
        RestartStep             = f_basic.RestartStep;

        sprintf(restart_name,"%s/intfc-ts%s",restart_name,
                        right_flush(RestartStep,7));
        if (pp_numnodes() > 1)
            sprintf(restart_name,"%s-nd%s",restart_name, 
                                right_flush(pp_mynode(),4));

	FT_StartUp(&front,&f_basic);

	if (!RestartRun)
	{
	    /* Initialize interface through level function */

	    circle_params.dim = 2;
	    circle_params.cen[0] = 0.5;
	    circle_params.cen[1] = 0.5;
	    circle_params.R = 0.3;
	    circle_params.add_plan_surf = NO;

	    level_func_pack.neg_component = 1;
	    level_func_pack.pos_component = 2;
	    level_func_pack.func_params = (POINTER)&circle_params;
	    level_func_pack.func = level_circle_func;
	    level_func_pack.wave_type = FIRST_PHYSICS_WAVE_TYPE;

	    FT_InitIntfc(&front,&level_func_pack);
	    if (f_basic.dim < 3)
                FT_ClipIntfcToSubdomain(&front);
	}

	/* Initialize velocity field function */

	dv_params.cen1[0] = 0.25;
	dv_params.cen2[0] = 0.75;
	dv_params.cen1[1] = 0.5;
	dv_params.cen2[1] = 0.5;
	dv_params.i1 = 1.0;
	dv_params.i2 = -1.0;
	dv_params.dim = 2;
	dv_params.reverse_time = HUGE;
	velo_func_pack.func_params = (POINTER)&dv_params;
	velo_func_pack.func = double_vortex_vel;
	velo_func_pack.point_propagate = first_order_point_propagate;

	FT_InitFrontVeloFunc(&front,&velo_func_pack);

	/* Propagate the front */

	test_propagate(&front);

	clean_up(0);
	return 0;
}


static  void test_propagate(
        Front *front)
{
        double CFL;
        int  dim = front->rect_grid->dim;

	front->max_time = 0.5;
	front->max_step = 800;
	front->print_time_interval = 0.1;
	front->movie_frame_interval = 0.025;

        CFL = Time_step_factor(front);

        printf("dim = %d\n", dim);
	printf("CFL = %f\n",CFL);
	printf("Frequency_of_redistribution(front,GENERAL_WAVE) = %d\n",
		Frequency_of_redistribution(front,GENERAL_WAVE));

	if (!RestartRun)
	{
	    FT_RedistMesh(front);
            FT_ResetTime(front);

	    // Always output the initial interface.
	    FT_Save(front);
            FT_Draw(front);

	    // This is a virtual propagation to get maximum front 
	    // speed to determine the first time step.

            FT_Propagate(front);
            FT_SetTimeStep(front);
            FT_SetOutputCounter(front);
	}
	else
            FT_SetOutputCounter(front);

	FT_TimeControlFilter(front);

        for (;;)
        {
	    /* Propagating interface for time step dt */

            FT_Propagate(front);
            FT_AddTimeStepToCounter(front);

	    //Next time step determined by maximum speed of previous
	    //step, assuming the propagation is hyperbolic and
	    //is not dependent on second order derivatives of
	    //the interface such as curvature, and etc.

            FT_SetTimeStep(front);

	    /* Output section */

	    if (FT_IsSaveTime(front))
                FT_Save(front);
            if (FT_IsDrawTime(front))
                FT_Draw(front);

            if (FT_TimeLimitReached(front))
	    {
		FT_PrintTimeStamp(front);
                break;
	    }

	    FT_TimeControlFilter(front);
	    FT_PrintTimeStamp(front);
        }
        (void) delete_interface(front->interf);
}       /* end test_propagate */

/********************************************************************
 *	Sample (circle) velocity function for the front    *
 ********************************************************************/

static int test_vel(
	POINTER params,
	Front *front,
	POINT *p,
	HYPER_SURF_ELEMENT *hse,
	HYPER_SURF *hs,
	double *vel)
{
	int i,dim = front->rect_grid->dim;
	double speed = 0.1*front->rect_grid->h[0];
	double *coords = Coords(p);
	double r = 0.0;
	for (i = 0; i < dim; ++i)
	    r += sqr(coords[0] - 0.5);
	r = sqrt(r);
	for (i = 0; i < dim; ++i)
	    vel[i] = coords[i]/r*speed;
}	/* end test_vortex_vel */
