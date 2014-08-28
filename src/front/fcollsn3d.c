/***************************************************************
FronTier is a set of libraries that implements differnt types of 
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


#include <front/fdecs.h>		/* includes int.h, table.h */

enum {
	UNKNOWN_COLLISION		=	-1,
	FABRIC_FABRIC_COLLISION		=	1,
	FABRIC_RIGID_COLLISION,
	RIGID_RIGID_COLLISION
};

LOCAL int collision_type(INTERFACE*,SURFACE**);
LOCAL void fabric_rigid_collision(Front*,C_CURVE*);
LOCAL void find_convex_tri_cluster(TRI**,int,TRI**,int*);
LOCAL void set_tr_tf_pf_list(Front*,C_CURVE*,TRI***,int*,TRI***,int*,
			POINT***,int*);
LOCAL void add_neighbors_of_landing_tri(TRI***,int*,SURFACE*,int*,TRI*);
LOCAL void check_add_tri(TRI***,TRI*,int*,int*);
LOCAL void check_add_pt(POINT***,POINT*,int*,int*);
LOCAL void test_rigid_pts(Front*,TRI**,int,TRI**,int);

EXPORT	void	resolve_collision(
	Front	*front)
{
	INTERFACE *intfc = front->interf;
	CROSS *cr,*cross;
	C_CURVE *cc;
	int i;
	SURFACE **s;
	int c_type;
	char fname[100];
	int count = 0;

	printf("Entering resolve_collision()\n");
	start_clock("intersection");
	intersections(intfc,&cross,NO);
	printf("cross = %p\n",(void*)cross);
	stop_clock("intersection");
	if (cross == NULL) return;
	/*
	while (cross != NULL)
	{
	*/
	    for (i = 0, cr = cross; cr != NULL; ++i, cr = cr->next)
	    {
		cc = cr->c_curve;
		sprintf(fname,"cross-%d",count);
		gview_plot_c_curve(cc,i,fname);
		s = cc->s;
		c_type = collision_type(intfc,s);
		switch (c_type)
		{
		case FABRIC_RIGID_COLLISION:
		    fabric_rigid_collision(front,cc);
		    break;
		case FABRIC_FABRIC_COLLISION:
		    (void) printf("FABRIC_FABRIC_COLLISION not implemented!\n");
		    continue;
		case RIGID_RIGID_COLLISION:
		    (void) printf("RIGID_RIGID_COLLISION not implemented!\n");
		    clean_up(ERROR);
		default:
		    (void) printf("Unknown collision!\n");
		    clean_up(ERROR);
		}
	    }
	/*
	}
	clean_up(0);
	*/
}	/* end resolve_collision */

LOCAL int collision_type(
	INTERFACE *intfc,
	SURFACE **s)
{
	int w_type1,w_type2;

	w_type1 = wave_type(s[0]);
	w_type2 = wave_type(s[1]);

	if (w_type1 == ELASTIC_BOUNDARY)
	{
	    if (w_type2 == ELASTIC_BOUNDARY)
		return FABRIC_FABRIC_COLLISION;
	    else if (w_type2 == NEUMANN_BOUNDARY ||
		w_type2 == MOVABLE_BODY_BOUNDARY)
		return FABRIC_RIGID_COLLISION;
	    else
		return UNKNOWN_COLLISION;
	}
	else if (w_type1 == NEUMANN_BOUNDARY ||
            w_type2 == MOVABLE_BODY_BOUNDARY)
	{
	    if (w_type2 == ELASTIC_BOUNDARY)
		return FABRIC_RIGID_COLLISION;
	    else if (w_type2 == NEUMANN_BOUNDARY ||
		w_type2 == MOVABLE_BODY_BOUNDARY)
		return RIGID_RIGID_COLLISION;
	    else
		return UNKNOWN_COLLISION;
	}
	else
	    return UNKNOWN_COLLISION;

}	/* end collision_type */

LOCAL void fabric_rigid_collision(
	Front *front,
	C_CURVE *cc)
{
	int i,j,k,num_trs,num_tfs,num_pfs;
	TRI **tr_list,**tf_list,*tri_on;
	POINT *p,**pf_list;
	double scaled_gap_tol = 0.05;
	double **newpts;
	SIDE status;
	SURFACE *sr = NULL;
	SURFACE *sf = NULL;
	double nearest_pt[MAXD],nor[MAXD];
	double hdir,*h = front->rect_grid->h;
	COMPONENT int_comp,ext_comp;
	POINT **pr_list;
	int num_prs,max_num_prs;
	int id;
	char dirname[200];

	if (debugging("collision"))
	    (void) printf("Entering fabric_rigid_collision()\n");

	for (i = 0; i < 2; ++i)
	{
	    if (wave_type(cc->s[i]) == NEUMANN_BOUNDARY ||
		wave_type(cc->s[i]) == MOVABLE_BODY_BOUNDARY)
		sr = cc->s[i];
	    if (wave_type(cc->s[i]) == ELASTIC_BOUNDARY)
		sf = cc->s[i];
	}
	if (sr == NULL)
	{
	    (void) printf("In fabric_rigid_collision(): ");
	    (void) printf("cannot find rigid body surface!\n");
	    clean_up(ERROR);
	}
	ext_comp = exterior_component(front->interf);
	int_comp = (positive_component(sr) == ext_comp) ? 
		    negative_component(sr) : positive_component(sr);

	set_tr_tf_pf_list(front,cc,&tr_list,&num_trs,&tf_list,&num_tfs,
				&pf_list,&num_pfs);

	FT_MatrixMemoryAlloc((POINTER*)&newpts,num_pfs,MAXD,sizeof(double));
	sprintf(dirname,"cross-tris/cross0-%d",front->step);
	gview_plot_crossing_tris(dirname,tr_list,num_trs,tf_list,num_tfs);
	for (i = 0; i < num_pfs; ++i)
	{
	    status = nearest_point_to_tri_cluster(Coords(pf_list[i]),int_comp,
                            sr,tr_list,num_trs,&tri_on,&id,nearest_pt,nor);
	    hdir = grid_size_in_direction(nor,h,3);
	    for (k = 0; k < 3; ++k)
                newpts[i][k] = nearest_pt[k] + scaled_gap_tol*hdir*nor[k];
	}
	for (i = 0; i < num_pfs; ++i)
	{
	    for (k = 0; k < 3; ++k)
		Coords(pf_list[i])[k] = newpts[i][k];
	}
	sprintf(dirname,"cross-tris/cross1-%d",front->step);
	gview_plot_crossing_tris(dirname,tr_list,num_trs,tf_list,num_tfs);
	printf("Component after treatment:\n");
	for (i = 0; i < num_pfs; ++i)
	{
	    COMPONENT comp;
	    p = pf_list[i];
	    comp = component_wrt_tri_cluster(Coords(p),sr,tr_list,
				num_trs);
	    printf("comp[%d] = %d\n",i,comp);
	}
	num_prs = max_num_prs = 0;
	pr_list = NULL;
	for (i = 0; i < num_trs; ++i)
	{
	    double v[MAXD];
	    for (k = 0; k < 3; ++k)
	    {
		p = Point_of_tri(tr_list[i])[k];
	    	status = nearest_point_to_tri_cluster(Coords(p),int_comp,
                            sf,tf_list,num_tfs,&tri_on,&id,nearest_pt,nor);
		for (j = 0; j < 3; ++j)
		{
		    v[j] = Coords(p)[j] - nearest_pt[j];
		}
		if (Dot3d(v,nor) > 0.0)
		    check_add_pt(&pr_list,p,&num_prs,&max_num_prs);
		printf("Dot[%d][%d] = %f\n",i,k,Dot3d(v,nor));
	    }
	}
	add_to_debug("tri_cluster");
	for (i = 0; i < num_prs; ++i)
	{
	    p = pr_list[i];
	    status = nearest_point_to_tri_cluster(Coords(p),int_comp,
                            sf,tf_list,num_tfs,&tri_on,&id,nearest_pt,nor);
	    printf("tri_on = %p\n",(void*)tri_on);
	    if (status == ONEDGE)
		printf("ie = %d\n",id);
	    else
		printf("\n");
	}
	remove_from_debug("tri_cluster");
	printf("num_prs = %d\n",num_prs);
	free_these(4,tr_list,tf_list,pf_list,newpts);
}	/* end fabric_rigid_collision */

LOCAL void test_rigid_pts(
	Front *front,
	TRI **tr_list,
	int num_trs,
	TRI **tf_list,
	int num_tfs)
{
	POINT *p,**pr_list;
	int i,j,k,num_prs;
	printf("Entering test_rigid_pts()\n");
	
}	/* end test_rigid_pts */

/*	This function set up the lists (1) tris on fabric surface,
 *	(2) tris on rigid surface, and (3) fabric points which 
 *	have crossed the rigid surface.
 */

LOCAL void set_tr_tf_pf_list(
	Front *front,
	C_CURVE *cc,
	TRI ***tr_list,
	int *num_trs,
	TRI ***tf_list,
	int *num_tfs,
	POINT ***pf_list,
	int *num_pfs)
{
	int i,j,k,num_cbs,num_tf,num_tr,nt;
	C_BOND *cb,**cbs;
	SURFACE *sr,*sf;	/* rigid and fabric surfaces */
	TRI **tr,**tf;		/* rigid and fabric tri lists */
	POINT *p,**pf;		/* fabric point lists */
	int npf,ntr,ntf;
	int max_npf,max_ntr,max_ntf;
	COMPONENT comp,ext_comp,int_comp;
			/* ext_comp is the component exterior to comp domain */
			/* int_comp is the component interior to comp domain */
	SIDE status;
	boolean convex_tri_added;
	TRI *t,*tr_on,*nbt1,*nbt2,**ptris;
	double nearest_pt[MAXD],nor[MAXD];
	int id;

	/* resetting */
	num_cbs = 0;
	sr = sf = NULL;
	tr = tf = NULL;
	pf = NULL;
	npf = ntf = ntr = 0;
	max_npf = max_ntf = max_ntr = 0;

	printf("Entering set_tr_tf_pf_list()\n");
	for (cb = cc->first; cb != NULL; cb = cb->next)
	    num_cbs++;
	printf("num_cbs = %d\n",num_cbs);

	for (i = 0; i < 2; ++i)
	{
	    if (wave_type(cc->s[i]) == NEUMANN_BOUNDARY ||
		wave_type(cc->s[i]) == MOVABLE_BODY_BOUNDARY)
		sr = cc->s[i];
	    if (wave_type(cc->s[i]) == ELASTIC_BOUNDARY)
		sf = cc->s[i];
	}
	ext_comp = exterior_component(front->interf);
	int_comp = (positive_component(sr) == ext_comp) ? 
		    negative_component(sr) : positive_component(sr);

	for (i = 0, cb = cc->first; cb != NULL; i++, cb = cb->next)
	{
	    for (k = 0; k < 2; ++k)
	    {
	    	t = cb->s[k].t;
	    	if (t->surf == sr) 
	    	    check_add_tri(&tr,t,&ntr,&max_ntr);
	    	if (t->surf == sf) 
	    	    check_add_tri(&tf,t,&ntf,&max_ntf);
	    }
	}
	printf("Step 1: ntr = %d  ntf = %d\n",ntr,ntf);
	convex_tri_added = YES;
	while (convex_tri_added)
	{
	    convex_tri_added = NO;
	    for (i = 0; i < ntr-1; ++i)
	    {
		for (k = 0; k < 3; ++k)
		{
		    nbt1 = Tri_on_side(tr[i],k);
		    if (!pointer_in_list((POINTER)nbt1,ntr,(POINTER*)tr))
			break;
		}
		if (k == 3) continue; /* all neighbors already in list */
	    	for (j = i+1; j < ntr; ++j)
		{
		    for (k = 0; k < 3; ++k)
		    {
		    	nbt2 = Tri_on_side(tr[j],k);
		    	if (pointer_in_list((POINTER)nbt2,ntr,(POINTER*)tr))
			    continue;
			if (nbt1 == nbt2)
	    	    	    check_add_tri(&tr,nbt1,&ntr,&max_ntr);
		    }
		}
	    }
	}
	convex_tri_added = YES;
	while (convex_tri_added)
	{
	    convex_tri_added = NO;
	    for (i = 0; i < ntf-1; ++i)
	    {
		for (k = 0; k < 3; ++k)
		{
		    nbt1 = Tri_on_side(tf[i],k);
		    if (!pointer_in_list((POINTER)nbt1,ntf,(POINTER*)tf))
			break;
		}
		if (k == 3) continue; /* all neighbors already in list */
	    	for (j = i+1; j < ntf; ++j)
		{
		    for (k = 0; k < 3; ++k)
		    {
		    	nbt2 = Tri_on_side(tf[j],k);
		    	if (pointer_in_list((POINTER)nbt2,ntf,(POINTER*)tf))
			    continue;
			if (nbt1 == nbt2)
	    	    	    check_add_tri(&tf,nbt1,&ntf,&max_ntf);
		    }
		}
	    }
	}
	printf("Step 2: ntr = %d  ntf = %d\n",ntr,ntf);
	for (i = 0; i < ntf; ++i)
	{
	    for (j = 0; j < 3; ++j)
	    {
		p = Point_of_tri(tf[i])[j];
		if (pointer_in_list((POINTER)p,npf,(POINTER*)pf))
		    continue;
		comp = component_wrt_tri_cluster(Coords(p),sr,tr,ntr);
		if (comp == ext_comp)
            	{
		    check_add_pt(&pf,p,&npf,&max_npf);
		    nt = set_tri_list_around_point(p,tf[i],&ptris,
				front->interf);
		    for (k = 0; k < nt; ++k)
			check_add_tri(&tf,ptris[k],&ntf,&max_ntf);
		}
		status = nearest_point_to_tri_cluster(Coords(p),int_comp,
                                sr,tr,ntr,&tr_on,&id,nearest_pt,nor);
		add_neighbors_of_landing_tri(&tr,&ntr,sr,&max_ntr,tr_on);
	    }
	}
	printf("Step 3: ntr = %d  ntf = %d  npf = %d\n",ntr,ntf,npf);
	*tr_list = tr;		*num_trs = ntr;
	*tf_list = tf;		*num_tfs = ntf;
	*pf_list = pf;		*num_pfs = npf;
}	/* end set_tr_tf_pf_list */

LOCAL void add_neighbors_of_landing_tri(
	TRI ***tri_list,
	int *num_tri,
	SURFACE *s,
	int *max_num_tri,
	TRI *tri)
{
	HYPER_SURF_ELEMENT *hse = Hyper_surf_element(tri);
        HYPER_SURF *hs = Hyper_surf(s);
	TRI *nbtris[20];
	int i,num_nbtri;

	TriAndFirstRing(hse,hs,&num_nbtri,nbtris);
	for (i = 0; i < num_nbtri; ++i)
	    check_add_tri(tri_list,nbtris[i],num_tri,max_num_tri);
}	/* add_neighbors_of_landing_tri */

LOCAL void check_add_tri(
	TRI ***tri_list,
	TRI *tri,
	int *num_tris,
	int *max_num_tris)
{
	if (*num_tris >= *max_num_tris)
	{
	    int i;
	    TRI **tri_store;
	    *max_num_tris += 100;
	    FT_VectorMemoryAlloc((POINTER*)&tri_store,*max_num_tris,
				sizeof(TRI*));
	    for (i = 0; i < *num_tris; ++i)
		tri_store[i] = (*tri_list)[i];
	    if (*tri_list != NULL)
	    	free_these(1,*tri_list);
	    *tri_list = tri_store;
	}
	if (!pointer_in_list((POINTER)tri,*num_tris,(POINTER*)*tri_list))
	{
	    (*tri_list)[*num_tris] = tri;
	    (*num_tris)++;
	}
}	/* end check_add_tri */

LOCAL void check_add_pt(
	POINT ***pt_list,
	POINT *pt,
	int *num_pts,
	int *max_num_pts)
{
	if (*num_pts >= *max_num_pts)
	{
	    int i;
	    POINT **pt_store;
	    *max_num_pts += 100;
	    FT_VectorMemoryAlloc((POINTER*)&pt_store,*max_num_pts,
				sizeof(POINT*));
	    for (i = 0; i < *num_pts; ++i)
		pt_store[i] = (*pt_list)[i];
	    if (*pt_list != NULL)
	    	free_these(1,*pt_list);
	    *pt_list = pt_store;
	}
	if (!pointer_in_list((POINTER)pt,*num_pts,(POINTER*)*pt_list))
	{
	    (*pt_list)[*num_pts] = pt;
	    (*num_pts)++;
	}
}	/* end check_add_pt */
