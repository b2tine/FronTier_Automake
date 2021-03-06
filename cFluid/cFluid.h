#include <FronTier.h>
#include <vector>
#include <list>
#include <assert.h>

#define EXT_COMP 0
#define SOLID_COMP 0
#define GAS_COMP1 2
#define GAS_COMP2 3
#define MAX_COMP 10

#define	gas_comp(comp) (((comp) == GAS_COMP1 ||	\
            comp == GAS_COMP2) ? YES : NO)

enum PROB_TYPE
{
    ERROR_TYPE = -1,
    TWO_FLUID_BUBBLE = 1,
    TWO_FLUID_RT,
    FLUID_SOLID_CIRCLE,
    BUBBLE_SURFACE,
    FLUID_RIGID_BODY,
    ROTOR_ONE_FLUID,
    ROTOR_TWO_FLUID,
    TWO_FLUID_RM,
    TWO_FLUID_RM_RAND,
    IMPLOSION,
    MT_FUSION,
    PROJECTILE,
    RIEMANN_PROB,
    FLUID_CRYSTAL,
    ONED_BLAST,
    ONED_SSINE,
    ONED_ASINE,
    FLUID_SOLID_RECT,
    FLUID_SOLID_TRIANGLE,
    FLUID_SOLID_CYLINDER,
    OBLIQUE_SHOCK_REFLECT
};

struct EOS_PARAMS 
{
    double  gamma;
    double  pinf;
    double  einf;
};

struct STATE
{
	int dim;
	double dens;
    double engy;
	double momn[MAXD];
	double pres;	
	double vel[MAXD];
	double vort;
	EOS_PARAMS* eos;
};

enum NUM_SCHEME
{
    TVD_FIRST_ORDER	= 1,
    TVD_SECOND_ORDER,
    TVD_FOURTH_ORDER,
    WENO_FIRST_ORDER,
    WENO_SECOND_ORDER,
    WENO_FOURTH_ORDER
};

enum POINT_PROP_SCHEME
{
    FIRST_ORDER	= 1,
    SECOND_ORDER,
    FOURTH_ORDER
};

enum SHOCK_PARAMETER
{
    BEHIND_PRESSURE = 1,
	BEHIND_VELOCITY,
	SHOCK_SPEED,
	SHOCK_MACH_NUMBER
};

struct FIELD
{
	double **vel;
	double **momn;
	double *dens;
	double *engy;
	double *pres;
	double *vort;
};

struct SWEEP
{
    double *dens;
    double **momn;
    double *engy;
    double *pres;
};

struct FSWEEP
{
    double *dens_flux;
    double **momn_flux;
    double *engy_flux;
};

struct GHOSTS
{
	double **gnor;
	double **Gdens;
	double ***Gvel;
	double **Gpres;
};

struct EQN_PARAMS
{
    int dim;
    PROB_TYPE prob_type;
    POINTER level_func_params;
	NUM_SCHEME num_scheme;
    POINT_PROP_SCHEME point_prop_scheme;
	EOS_PARAMS      eos[MAX_COMP];
	boolean tracked;
	boolean articomp;
	boolean contact_stationary;

    int idir;
	int shock_side;

    //pressure
    double p0;
    double p1;
    double p2;
    double p3;

    //density
    double rho0;
    double rho1;
    double rho2;
    double rho3;

    //velocity
    double v0[MAXD];
    double v1[MAXD];
    double v2[MAXD];
    double v3[MAXD];

    double min_dens;
    double min_pres;

    double mu1;
	double mu2;
	double gamma;
	double gravity[MAXD];
	double Mach_number;
	double shock_position;
	double contact_vel;

    //state variables
        //FIELD eqnField;
	double **vel;
	double **mom;
	double *dens;
	double *engy;
	double *pres;
	double *vort;

    //GFM variables
        //GHOSTS ghosts;
	double **gnor;
	double **Gdens;
	double ***Gvel;
	double **Gpres;

    //TODO: This may warrant a seperate structure.
	boolean use_base_soln;
    char base_dir_name[200];
    int num_step;
    int *steps;
    F_BASIC_DATA *f_basic;

};

struct SCHEME_PARAMS
{
	boolean artificial_compression;
    double lambda;
    double beta;
	double gamma;
    double einf;
};

struct FLOW_THROUGH_PARAMS
{
    POINT *oldp;
    COMPONENT comp;
    EQN_PARAMS *eqn_params;
};

struct RG_PARAMS
{
    int dim;
    boolean no_fluid;
    double  total_mass;
    double  moment_of_inertia;
    double  center_of_mass[MAXD];
    double  rotation_dir[MAXD];
    double  translation_dir[MAXD];
    double  rotation_cen[MAXD];
    double  cen_of_mass_velo[MAXD];
    double  angular_velo;
    double  p_moment_of_inertia[MAXD];
    double  p_angular_velo[MAXD];
    double  euler_params[4];
    double  old_euler_params[4];
    MOTION_TYPE motion_type;
};


struct VAR_BDRY_PARAMS
{
    int dim;
    double center[MAXD];
    double *angles_pistons;     /* Angles to the pistons' centers */
    double half_angular_width; /* Half-angle of the piston's surface */
    double bdry_vel;
    double bdry_dens;
    double bdry_pres;
    int number_pistons;
    double jet_duration_time;
};

/*********************************************************************
*	Definition and structures for Riemann solution.              *
**********************************************************************/

#define GAMMA_PLUS 2
#define GAMMA_MINUS 3
#define LF_SHOCK 4
#define RF_SHOCK 5
#define CONTACT 6
#define VACUUM 7

struct RIEM_STATE
{
    double d;
    double p;
    double u;
    double gamma;
};

struct CENTERED_WAVE
{
    int wave_type;
    double speed_leading_edge;
    double speed_trailing_edge;
    double speed_shock;
    double speed_contact;
};

struct RIEMANN_SOLN
{
    RIEM_STATE left_state;
    RIEM_STATE right_state;
    RIEM_STATE left_center_state;
    RIEM_STATE right_center_state;
    CENTERED_WAVE left_wave;
    CENTERED_WAVE right_wave;
    CENTERED_WAVE contact;
};

struct RIEMANN_INPUT
{
    RIEM_STATE left_state;
    RIEM_STATE right_state;
};

struct OPEN_PIPE_PARAMS
{
	int dir;
	int side;
	double center[MAXD];
	double radius;
	int in_pipe_bdry;
	int out_pipe_bdry;
	boolean in_flow_through;
	boolean out_flow_through;
	STATE state[2];
};

class L_RECTANGLE
{
    public:
	
        int m_index;    // rectangle index
	    int comp;			 
	    double m_coords[MAXD];	
	    int icoords[MAXD];

	    L_RECTANGLE();

	    void setCoords(double*,int);
};


class G_CARTESIAN
{

public:

	explicit G_CARTESIAN(Front* frnt);
	
    ~G_CARTESIAN();

    G_CARTESIAN() = delete;
    G_CARTESIAN(const G_CARTESIAN&) = delete;
    G_CARTESIAN(G_CARTESIAN&&) = delete;
    G_CARTESIAN& operator=(const G_CARTESIAN&) = delete;
    G_CARTESIAN& operator=(G_CARTESIAN&&) = delete;

	int dim;
	double max_dt;			// max_dt from cartesian
	double hmin;			// smallest spacing

	void initMesh(void);		// setup the cartesian grid
    void setInitialStates();
	void readInteriorStates(char*);
	void printFrontInteriorStates(char*);
	void initMovieVariables(); //TODO: move this out of class
	void getVelocity(double*,double*);
	void compareWithBaseData(char *out_name);
	void freeBaseFront();
	void errFunction();
	void solve(); // main step function

private:

	Front* front;
	RECT_GRID *top_grid;    //NOTE: top means topological
	COMPONENT* top_comp;
	EQN_PARAMS* eqn_params;
	FIELD field;
	FIELD* base_field;
	Front* base_front;
	double* array;  // for scatter states;

	int imin[MAXD]; // Sweeping limits
	int imax[MAXD];
	int top_gmax[MAXD];
	int lbuf[MAXD],ubuf[MAXD];
	double top_L[MAXD],top_U[MAXD],top_h[MAXD];
    void getDomainBounds();

	int **ij_to_I,**I_to_ij;	// Index mapping for 2D
	int ***ijk_to_I,**I_to_ijk;	// Index mapping for 3D
    
	int nrad {3};   // Buffer size for a given numerical scheme


	// mesh storage
	std::vector<L_RECTANGLE> cell_center;

    //NOTE: Can only support 2-component flows
	int m_comp[2];
	double m_mu[2];
	double m_dens[2];
	double m_smoothing_radius;

	double m_t; //time
	double max_speed;//for stability of convection

	//minimum physical variables
	const double min_dens {0.0001};
    const double min_pres {0.0001};

	//domain parallel partitioning
	int NLblocks;
    int ilower;
    int iupper;
    int* n_dist;

	//mesh
	void setDomain();
	void setComponent();
	void augmentMovieVariables();
	void copyMeshStates();
	void sampleVelocity();
	void sampleVelocity2d();
	void sampleVelocity3d();

    /*TMP*/
	void checkVst(SWEEP*);
	void checkFlux(FSWEEP*);

	//domain parallelization routines via MPI
	void scatMeshStates();
	void scatMeshVst(SWEEP*);
	void scatMeshFlux(FSWEEP*);
	void appendOpenEndStates();


	// -------------------------------------------------------
	// 		compressible solver functions
	// -------------------------------------------------------
     
	void setAdvectionDt();
	void computeAdvection();

	/* Mesh memory management */
    int sizeEqnVst;
    void initComputationalData();
    void allocEqnField();

	void allocMeshVst(SWEEP*);
	void allocMeshFlux(FSWEEP*);
	void allocDirVstFlux(SWEEP*,FSWEEP*);
	
    void freeVst(SWEEP*);
	void freeFlux(FSWEEP*);
	void freeDirVstFlux(SWEEP*,FSWEEP*);

	bool withinStencilLen(int*,int);

    /* Runge Kutta */ 
    int orderRK;
    std::vector<double> weightsRK;
    std::vector<std::vector<double> > coeffsRK;
    SWEEP* st_field;
    FSWEEP* st_flux;

	void initRungeKutta();
	void setFirstOrderRK();
	void setSecondOrderRK();
	void setFourthOrderRK();
    void allocRungeKuttaVstFlux();
	void solveRungeKutta();

	/* Mesh operations */
	void addMeshFluxToVst(SWEEP*,FSWEEP,double);
	void computeMeshFlux(SWEEP,FSWEEP*,double);
	void copyMeshVst(SWEEP,SWEEP*);
	void copyFromMeshVst(SWEEP);
	void copyToMeshVst(SWEEP*);
	void addSourceTerm(SWEEP*,FSWEEP*,double);

	/* Directional flux solver */
	void resetFlux(FSWEEP*);
	void addFluxInDirection(int,SWEEP*,FSWEEP*,double);
	void addFluxAlongGridLine(int,int*,double,SWEEP*,FSWEEP*);
	void augmentOneDimBuffer(int,int);
	void numericalFlux(POINTER,SWEEP*,FSWEEP*,int);
	void appendStencilBuffer2d(SWEEP*,SWEEP*,int,int);
	void appendStencilBuffer3d(SWEEP*,SWEEP*,int,int,int);
	void appendGhostBuffer(SWEEP*,SWEEP*,int,int*,int,int);

    // -------------------------------------------------------
	// 		initialization functions
	// -------------------------------------------------------

    //TODO: move these out of class if possible
	void initRayleighTaylorStates();
	void initRichtmyerMeshkovStates();
	void initBubbleStates();
	void initImplosionStates();
	void initMTFusionStates();
	void initProjectileStates();
	void initRiemProbStates();
	void initBlastWaveStates();
	void initShockSineWaveStates();
	void initAccuracySineWaveStates();
	void initObliqueStates();

    void readBaseFront(int i);
	void readBaseStates(char *restart_name);
	void readFrontInteriorStates(char *restart_state_name);

	void compSGS(void); //subgrid model by Hyunkyung Lim

	void getPressureJumpParameter(double *coords0, double *coords1,
            double &theta, double &jumpPressure, double &jumpDerivative);

	// velocity field query
	void getVelocityGradient(double *p,double *gradU,double *gradV);

	// ----------------------------------------------------------
	// 		utility functions
	// ----------------------------------------------------------

	void getRectangleIndex(int indexRectangle, int &i, int &j);
	void getRectangleIndex(int indexRectangle, int &i, int &j, int &k);
	int getRectangleComponent(int index);	// the center component
	void getRectangleCenter(int index, double *coords);
	void getRectangleCenter(int index0, int index1, double *coords);
	
	int getInteger(double i);
	boolean isInteger(double i);

	double getVorticity(int i, int j);
    double getVorticityX(int i, int j, int k);
    double getVorticityY(int i, int j, int k);
    double getVorticityZ(int i, int j, int k);
	double getDistance(double *coords0, double *coords1);

    // TODO: Investigate to what degree the
    //       function getNearestInterfacePoint()
    //       has been implemented.
        
    //CLAIM: incompletely implemented
	void getNearestInterfacePoint(double *q,double *p);
		
	int  getComponent(int *icoords);	
	int  getComponent(double *coords);	
	void save(char *filename);

	void setDirichletStates(STATE*,SWEEP*,SWEEP*,
            HYPER_SURF*,int*,int,int,int,int);
	void setNeumannStates(SWEEP*,SWEEP*,HYPER_SURF*,
            STATE*,int*,int,int,int,int,int);

	//GFM
	void solve_exp_value();
	boolean get_ave_normal(int*,int***);
	boolean get_ave_state(SWEEP, int*,int***,int,int);
	boolean needBufferFromIntfc(COMPONENT,COMPONENT);
	void get_normal_from_front();
	void get_ghost_state(SWEEP, int,int);
	void tecplot_interior_states(char*);
	void scatMeshGhost();
	void GFMGhostState(int*,int,STATE*);
	void checkCorrectForTolerance(STATE*);
	void adjustGhostFluidStates();
    void setGhostFluidStatesToZero();
    void allocGhostFluid();
    void setupSolver();

};

void initSampleVelocity(Front*,char*);


//cFsub.cpp
extern double getStateDens(POINTER);
extern double getStateEngy(POINTER);
extern double getStateXmom(POINTER);
extern double getStateYmom(POINTER);
extern double getStateZmom(POINTER);
extern double getStatePres(POINTER);
extern double getStateVort(POINTER);
extern double getStateXvel(POINTER);
extern double getStateYvel(POINTER);
extern double getStateZvel(POINTER);
extern double getStateXvort(POINTER);
extern double getStateYvort(POINTER);
extern double getStateZvort(POINTER);
extern double burger_flux(double,double,double);
extern double linear_flux(double,double,double,double);
extern void read_dirichlet_bdry_data(char*,Front*);
extern void read_open_end_bdry_data(char*,Front*);
extern void restart_set_dirichlet_bdry_function(Front*);
extern void cF_flowThroughBoundaryState(double*,HYPER_SURF*,Front*,POINTER,
                        POINTER);
extern void cF_variableBoundaryState(double*,HYPER_SURF*,Front*,POINTER,
                        POINTER);
extern void cFluid_point_propagate(Front*,POINTER,POINT*,POINT*,
                        HYPER_SURF_ELEMENT*,HYPER_SURF*,double,double*);

//rgbody.c
extern void init_moving_bodies(EQN_PARAMS,char*,Front*);
extern void cfluid_compute_force_and_torque(Front*,HYPER_SURF*,double,double*,
			double*);
extern void record_moving_body_data(char*,Front*);
extern void readFrontStates(Front*,char*);
extern void reflectVectorThroughPlane(double*,double*,double*,int);
extern boolean reflectNeumannState(Front*,HYPER_SURF*,double*,COMPONENT,SWEEP*,
			STATE*);
extern double EosPressure(STATE*);
extern double EosInternalEnergy(STATE*);
extern double EosEnergy(STATE*);
extern double EosSoundSpeed(STATE*);
extern double EosSoundSpeedSqr(STATE*);
extern double EosMaxBehindShockPres(double,STATE*);
extern void   EosSetTVDParams(SCHEME_PARAMS*,EOS_PARAMS*);
extern void   CovertVstToState(STATE*,SWEEP*,EOS_PARAMS*,int,int);
extern void   findGhostState(STATE,STATE,STATE*);

//cFinit.cpp
extern void prompt_for_rigid_body_params(int,char*,RG_PARAMS*);
extern void set_rgbody_params(RG_PARAMS,HYPER_SURF*);
extern void insert_objects(Front*);
extern void read_cFluid_params(char*,EQN_PARAMS*);
extern void cFluid_InitIntfc(Front*,LEVEL_FUNC_PACK*);

//cFriem.cpp
extern boolean RiemannSolution(RIEMANN_INPUT,RIEMANN_SOLN*);
extern boolean RiemannSolnAtXi(RIEMANN_SOLN*,RIEM_STATE*,double);
extern void TVD_flux(POINTER,SWEEP*,FSWEEP*,int);
extern void WENO_flux(POINTER,SWEEP*,FSWEEP*,int,int);


class EOS
{
    private:
        EOS_PARAMS *params;
    public:
        EOS(EOS_PARAMS &params);
	    double Pressure(STATE);
	    double Energy(STATE);
};

class COMPRESSIBLE_GAS_SOLVER
{
    private:
        
        Front *front;
        COMPONENT *top_comp;
        double *top_h;
        int *top_gmax;
        int imin,jmin,kmin;
        int imax,jmax,kmax;
        int dim;

    public: 

	    COMPRESSIBLE_GAS_SOLVER(Front &front);

        void set_solver_domain(void);
        void runge_kutta(void);
        void solve(void);
        void solve1d(void);
        void solve2d(void);
        void solve3d(void);

};


