//Include relevant C++ headers

#include <iostream> // input/output
#include <sstream> // handles string streams
#include <string>
#include <map>
#include <set>
#include <ctime> // used for random seed
#include <sys/stat.h> //to use mkdir

//include relevant MERLIN headers
#include "AcceleratorModel/ApertureSurvey.h"
#include "AcceleratorModel/ControlElements/Klystron.h"

#include "BeamDynamics/ParticleTracking/ParticleBunchConstructor.h"
#include "BeamDynamics/ParticleTracking/ParticleTracker.h"
#include "BeamDynamics/ParticleTracking/ParticleBunchTypes.h"
#include "BeamDynamics/ParticleTracking/Integrators/SymplecticIntegrators.h"
#include "BeamDynamics/TrackingSimulation.h"
#include "BeamDynamics/TrackingOutputAV.h"

#include "Collimators/CollimatorSurvey.h"
#include "Collimators/CollimateProtonProcess.h"
#include "Collimators/ScatteringProcess.h"
#include "Collimators/ScatteringModel.h"
#include "Collimators/CollimatorDatabase.h"
#include "Collimators/MaterialDatabase.h"
#include "Collimators/ApertureConfiguration.h"
#include "Collimators/Dustbin.h"
#include "Collimators/FlukaLosses.h"

#include "MADInterface/MADInterface.h"

#include "NumericalUtils/PhysicalUnits.h"
#include "NumericalUtils/PhysicalConstants.h"

#include "Random/RandomNG.h"

#include "RingDynamics/Dispersion.h"

// C++ std namespace, and MERLIN PhysicalUnits namespace

using namespace std;
using namespace PhysicalUnits;

// Main function, this executable can be run with the arguments number_of_particles seed
//e.g. for 1000 particles and a seed of 356: ./test 1000 356

int main(int argc, char* argv[])
{
    int seed = (int)time(NULL);		// seed for random number generators
    int iseed = (int)time(NULL);	// seed for random number generators
    int npart = 1;					// number of particles to track
    int nturns = 1;					// number of turns to track
	bool DoTwiss = 1;				// run twiss and align to beam envelope etc?
	 
    //~ if (argc >=2){npart = atoi(argv[1]);}
    if (argc >=3){seed = atoi(argv[2]);}

    RandomNG::init(iseed);

    double beam_energy = 6500.0;

    cout << "npart=" << npart << " nturns=" << nturns << " beam energy = " << beam_energy << endl;

	string start_element = "TCP.C6R7.B2";   // HORIZONTAL COLLIMATOR (x)
	    
    // Define useful variables
    double beam_charge = 1.1e11;
    double normalized_emittance = 3.5e-6;
    double gamma = beam_energy/PhysicalConstants::ProtonMassMeV/PhysicalUnits::MeV;
	double beta = sqrt(1.0-(1.0/pow(gamma,2)));
	double emittance = normalized_emittance/(gamma*beta);	
	//~ string directory = "/afs/cern.ch/user/h/harafiqu/public/MERLIN";					//lxplus harafiqu
	//~ string directory = "/home/haroon/git/Merlin/HR";									//iiaa1
	string directory = "/home/HR/Downloads/MERLIN_HRThesis/MERLIN";							//HR MAC
	//~ string directory = "/afs/cern.ch/work/a/avalloni/private/MerlinforFluka/MERLIN";	//lxplus avalloni	
	string input_dir = "/Thesis/data/6p5TeV/";
	//~ string input_dir = "/Thesis/data/AV/";
	
	string pn_dir, case_dir, bunch_dir, lattice_dir, fluka_dir, dustbin_dir;			
	
	string output_dir = "/Build/Thesis/outputs/6p5TeV/";
	string batch_directory="22_May_Aperture/";
	 
	string full_output_dir = (directory+output_dir);
	mkdir(full_output_dir.c_str(), S_IRWXU);
	
	full_output_dir = (directory+output_dir+batch_directory);
	mkdir(full_output_dir.c_str(), S_IRWXU);
	
	fluka_dir = full_output_dir + "Fluka/";    
	mkdir(fluka_dir.c_str(), S_IRWXU); 
	
	bool every_bunch			= 1;		// output whole bunch every turn in a single file
	bool rf_test				= 1;		// Use RF distribution to plot RF bucket
	bool output_initial_bunch 	= 0;
	bool output_final_bunch 	= 0;
		if (output_initial_bunch || output_final_bunch || every_bunch){
			bunch_dir = (full_output_dir+"Bunch_Distn/"); 	mkdir(bunch_dir.c_str(), S_IRWXU); 		
		}	
	
	bool output_fluka_database 	= 0;
	bool output_twiss			= 0;		
		if(output_twiss){ lattice_dir = (full_output_dir+"LatticeFunctions/"); mkdir(lattice_dir.c_str(), S_IRWXU); }	
	
	bool collimation_on 		= 0;
		if(collimation_on){
			dustbin_dir = full_output_dir + "LossMap/"; 	mkdir(dustbin_dir.c_str(), S_IRWXU);		
		}		
	bool use_sixtrack_like_scattering = 0;
	bool scatterplot			= 0;
	bool jawinelastic			= 0;
	bool jawimpact				= 0;
	
	bool ap_survey				= 1;
	bool coll_survey			= 0;
	bool output_particletracks	= 0;
	
	bool cleaning				= 0;
		if(cleaning){
			collimation_on		= 1;
			every_bunch			= 0;	
			output_initial_bunch= 0;
			output_final_bunch	= 0;
		}
	
	bool symplectic				= 1;
	bool sixD					= 1;
	
	bool CollMat				= 0;	// 0 = pure, 1 = composite
	
/************************************
*	ACCELERATORMODEL CONSTRUCTION	*
************************************/
	cout << "MADInterface" << endl;

	MADInterface* myMADinterface = new MADInterface( directory+input_dir+"Twiss_6p5TeV_flat_top_beam2.tfs", beam_energy );
	cout << "MADInterface Done" << endl;

    //~ myMADinterface->TreatTypeAsDrift("RFCAVITY");
    //~ myMADinterface->TreatTypeAsDrift("SEXTUPOLE");
    //~ myMADinterface->TreatTypeAsDrift("OCTUPOLE");

    myMADinterface->ConstructApertures(false);

    AcceleratorModel* myAccModel = myMADinterface->ConstructModel();    
	
	std::vector<RFStructure*> RFCavities;
	myAccModel->ExtractTypedElements(RFCavities,"ACS*");
	Klystron* Kly1 = new Klystron("KLY1",RFCavities);
	Kly1->SetVoltage(0.0);
	//~ Kly1->SetPhase(pi/2); 
	Kly1->SetPhase(pi/4); 
	
/************
*	TWISS	*
************/

    int start_element_number = myAccModel->FindElementLatticePosition(start_element.c_str());
    
    cout << "Found start element TCP.C6R at element number " << start_element_number << endl;

	LatticeFunctionTable* myTwiss = new LatticeFunctionTable(myAccModel, beam_energy);
	myTwiss->UseDefaultFunctions();
	myTwiss->AddFunction(1,6,3);
    myTwiss->AddFunction(2,6,3);
    myTwiss->AddFunction(3,6,3);
    myTwiss->AddFunction(4,6,3);
    myTwiss->AddFunction(6,6,3);
    
    double bscale1 = 1e-22;    
    if(DoTwiss){    
		while(true)
		{
		cout << "start while(true) to scale bend path length" << endl;
			// If we are running a lattice with no RF, the TWISS parameters
			// will not be calculated correctly. This is because some are
			// calculated from using the eigenvalues of the one turn map,
			// which is not complete with RF (i.e. longitudinal motion) 
			// switched off. In order to compensate for this we use the 
			// ScaleBendPath function which calls a RingDeltaT process 
			// and attaches it to the TWISS tracker. RingDeltaT process 
			// adjusts the ct and dp values such that the TWISS may be 
			// calculated and there are no convergence errors
			myTwiss->ScaleBendPathLength(bscale1);
			myTwiss->Calculate();

			// If Beta_x is a number (as opposed to -nan) then we have 
			// calculated the correct TWISS parameters, otherwise the loop
			//  keeps running
			if(!std::isnan(myTwiss->Value(1,1,1,0))) {break;}
			bscale1 *= 2;
			cout << "\n\ttrying bscale = " << bscale1<< endl;
		}
	}
	
	Dispersion* myDispersion = new Dispersion(myAccModel, beam_energy);
    myDispersion->FindDispersion(start_element_number);
	
	if (output_twiss){
		ostringstream twiss_output_file; 
		twiss_output_file << (lattice_dir+"LatticeFunctions.dat");
		ofstream twiss_output(twiss_output_file.str().c_str());
		if(!twiss_output.good()){ std::cerr << "Could not open twiss output file" << std::endl; exit(EXIT_FAILURE); } 
		myTwiss->PrintTable(twiss_output);
				
		ostringstream disp_output_file; 
		disp_output_file << (lattice_dir+"Dispersion.dat");
		ofstream* disp_output = new ofstream(disp_output_file.str().c_str());
		if(!disp_output->good()){ std::cerr << "Could not open dispersion output file" << std::endl; exit(EXIT_FAILURE); } 
		myDispersion->FindRMSDispersion(disp_output);
		delete disp_output;
	}
	
	if(sixD)
	Kly1->SetVoltage(2.0);
	
/************************
*	Collimator set up	*
************************/

	cout << "Collimator Setup" << endl;   
    
    MaterialDatabase* myMaterialDatabase = new MaterialDatabase();    
    CollimatorDatabase* collimator_db;
    if(CollMat)
		collimator_db = new CollimatorDatabase( directory+input_dir+"CollDB_Composite.txt", myMaterialDatabase,  true);
    else
		collimator_db = new CollimatorDatabase( directory+input_dir+"CollDB_Pure.txt", myMaterialDatabase,  true);
   
    collimator_db->MatchBeamEnvelope(true);
    collimator_db->UseMiddleJawHalfGap();
    collimator_db->EnableJawAlignmentErrors(false);

    collimator_db->SetJawPositionError(0.0 * nanometer);
    collimator_db->SetJawAngleError(0.0 * microradian);
    collimator_db->SelectImpactFactor(start_element, 1.0e-6);
    
    double impact = 6;
    // Finally we set up the collimator jaws to appropriate sizes
    try{
		if(DoTwiss)
        impact = collimator_db->ConfigureCollimators(myAccModel, emittance, emittance, myTwiss);
		else
        collimator_db->ConfigureCollimators(myAccModel);
    }
	catch(exception& e){ std::cout << "Exception caught: " << e.what() << std::endl; exit(1); }
    if(std::isnan(impact)){ cerr << "Impact is nan" << endl; exit(1); }
    cout << "Impact factor number of sigmas: " << impact << endl;
    
    if(output_fluka_database && seed == 1){
		ostringstream fd_output_file;
		fd_output_file << (full_output_dir+"fluka_database.txt");

		ofstream* fd_output = new ofstream(fd_output_file.str().c_str());
		collimator_db->OutputFlukaDatabase(fd_output);
		delete fd_output;
	}
    delete collimator_db;

/****************************
*	Aperture Configuration	*
****************************/

	ApertureConfiguration* myApertureConfiguration = new ApertureConfiguration(directory+input_dir+"Aperture_6p5TeV_beam2.tfs",1);      
    
    myApertureConfiguration->ConfigureElementApertures(myAccModel);
    delete myApertureConfiguration;

	if(ap_survey){
		ApertureSurvey* myApertureSurvey = new ApertureSurvey(myAccModel, full_output_dir, 0.01, 0);
	}
		
	if(coll_survey){
		CollimatorSurvey* CollSurvey = new CollimatorSurvey(myAccModel, emittance, emittance, myTwiss); 
		ostringstream cs_output_file;
		cs_output_file << full_output_dir << "coll_survey.txt";
		ofstream* cs_output = new ofstream(cs_output_file.str().c_str());
		if(!cs_output->good()) { std::cerr << "Could not open collimator survey output" << std::endl; exit(EXIT_FAILURE); }   
		CollSurvey->Output(cs_output, 20);			
		delete cs_output;
	}
	
/********************
*	Beam Settings	*
********************/

    BeamData mybeam;

    mybeam.charge = beam_charge/npart;
    mybeam.p0 = beam_energy;
    mybeam.beta_x = myTwiss->Value(1,1,1,start_element_number)*meter;
    mybeam.beta_y = myTwiss->Value(3,3,2,start_element_number)*meter;
    mybeam.alpha_x = -myTwiss->Value(1,2,1,start_element_number);
    mybeam.alpha_y = -myTwiss->Value(3,4,2,start_element_number);
    
    // Minimum and maximum sigma for HEL Halo Distribution
    mybeam.min_sig_x = 5.5;
    mybeam.max_sig_x = 5.54;
    //~ mybeam.min_sig_y = 0;
    //~ mybeam.max_sig_y = 3;
   
    // Dispersion
    mybeam.Dx=myDispersion->Dx;
    mybeam.Dy=myDispersion->Dy;
    mybeam.Dxp=myDispersion->Dxp;
    mybeam.Dyp=myDispersion->Dyp;

    mybeam.emit_x = emittance * meter;
    mybeam.emit_y = emittance * meter;


    //Beam centroid
    mybeam.x0=myTwiss->Value(1,0,0,start_element_number);
    mybeam.xp0=myTwiss->Value(2,0,0,start_element_number);
    mybeam.y0=myTwiss->Value(3,0,0,start_element_number);
    mybeam.yp0=myTwiss->Value(4,0,0,start_element_number);
    mybeam.ct0=myTwiss->Value(5,0,0,start_element_number);

    mybeam.sig_z = ((2.51840894498383E-10 * 299792458)) * meter;
    mybeam.sig_dp =  (1.129E-4);

    // X-Y coupling
    mybeam.c_xy=0.0;
    mybeam.c_xyp=0.0;
    mybeam.c_xpy=0.0;
    mybeam.c_xpyp=0.0;

    delete myDispersion;

/************
*	BUNCH	*
************/

    ProtonBunch* myBunch;
    ParticleBunchConstructor* myBunchCtor;
    int node_particles = npart;

	if(rf_test){
		//~ myBunchCtor = new ParticleBunchConstructor(mybeam, node_particles, RFDistn);
		myBunchCtor = new ParticleBunchConstructor(mybeam, node_particles, RFDistn2);
	}
	else{
		myBunchCtor = new ParticleBunchConstructor(mybeam, node_particles, Halo6p5TeV);
		//~ myBunchCtor = new ParticleBunchConstructor(mybeam, node_particles, Halo7TeV);
    }
    
    myBunch = myBunchCtor->ConstructParticleBunch<ProtonBunch>();
    delete myBunchCtor;

    myBunch->SetMacroParticleCharge(mybeam.charge);
 
	if(output_initial_bunch){   
		ostringstream hbunch_output_file;
		hbunch_output_file << bunch_dir << seed << "_initial_bunch.txt";
		ofstream* hbunch_output = new ofstream(hbunch_output_file.str().c_str());
		if(!hbunch_output->good()) { std::cerr << "Could not open initial halo bunch output" << std::endl; exit(EXIT_FAILURE); }   
		myBunch->Output(*hbunch_output);			
		delete hbunch_output;	
	}    

/************************
*	ParticleTracker		*
************************/

	AcceleratorModel::RingIterator beamline = myAccModel->GetRing(start_element_number);
    ParticleTracker* myParticleTracker = new ParticleTracker(beamline, myBunch);
    if(symplectic)
		myParticleTracker->SetIntegratorSet(new ParticleTracking::SYMPLECTIC::StdISet());
    else
		myParticleTracker->SetIntegratorSet(new ParticleTracking::TRANSPORT::StdISet());
    
	string tof = "Tracking_output_file.txt";
    string t_o_f = full_output_dir+tof;
    
	if(output_particletracks){
		ostringstream trackingparticles_sstream;
		trackingparticles_sstream << full_output_dir << "Tracking_output_file_"<< npart << "_" << seed << std::string(".txt");     
		string trackingparticles_file = trackingparticles_sstream.str().c_str();     
		 
		TrackingOutputAV* myTrackingOutputAV = new TrackingOutputAV(trackingparticles_file);
		myTrackingOutputAV->SetSRange(0, 27000);
		myTrackingOutputAV->SetTurn(1);
		myTrackingOutputAV->output_all = 1;
		 
		myParticleTracker->SetOutput(myTrackingOutputAV);
	}
	
/****************************
*	Collimation Process		*
****************************/

	//~ FlukaLosses* myFlukaLosses = new FlukaLosses;  
	LossMapDustbin* myLossMapDustbin = new LossMapDustbin;
	ScatteringModel* myScatter = new ScatteringModel;

	if(collimation_on){
		CollimateProtonProcess* myCollimateProcess =new CollimateProtonProcess(2, 4, NULL);
				
		myLossMapDustbin->Beam2(1);
		myCollimateProcess->SetDustbin(myLossMapDustbin);   	        
		//~ myCollimateProcess->SetFlukaLosses(myFlukaLosses); 
		
		myCollimateProcess->ScatterAtCollimator(true);
		
		if(scatterplot)	{	
		//~ myScatter->SetScatterPlot("TCP.C6R7.B2");
		myScatter->SetScatterPlot("TCSG.B5L7.B2");
		
		}
		if(jawimpact){
		//~ myScatter->SetJawImpact("TCP.C6R7.B2");
		myScatter->SetJawImpact("TCSG.B5L7.B2");
		
		}
		if(jawinelastic){
		myScatter->SetJawInelastic("TCP.C6R7.B2");  
		myScatter->SetJawInelastic("TCSG.B5L7.B2");  
		   
		}

		if(use_sixtrack_like_scattering){myScatter->SetScatterType(0);}
		else{myScatter->SetScatterType(4);}

		myCollimateProcess->SetScatteringModel(myScatter);
		myCollimateProcess->SetLossThreshold(200.0);
		myCollimateProcess->SetOutputBinSize(0.1);

		myParticleTracker->AddProcess(myCollimateProcess);
	}
	
/********************
 *  TRACKING RUN	*
 *******************/

	ostringstream cbo_file;
	cbo_file << bunch_dir << "Every_bunch.txt";
	ofstream* cboclean = new ofstream(cbo_file.str().c_str(), ios::trunc);
	ofstream* cbo = new ofstream(cbo_file.str().c_str(), ios::app);	
	if(!cbo->good())	{ std::cerr << "Could not open every bunch output file" << std::endl; exit(EXIT_FAILURE); }

	
	for (int turn=1; turn<=nturns; turn++)
    {
        cout << "Turn " << turn <<"\tParticle number: " << myBunch->size() << endl;

        myParticleTracker->Track(myBunch);
        
        if(every_bunch){myBunch->Output(*cbo); }         

        if( myBunch->size() <= 1 ) break;
    }
	
	/*********************************************************************
	**	Output Jaw Impact, Scatter Plot, Jaw Inelastic
	*********************************************************************/
	myScatter->OutputJawImpact(full_output_dir,seed);
	myScatter->OutputScatterPlot(full_output_dir,seed);	
    myScatter->OutputJawInelastic(full_output_dir,seed);
   
  	/*********************************************************************
	** OUTPUT FLUKA LOSSES  
	*********************************************************************/
	//~ ostringstream fluka_file;
	//~ fluka_file << fluka_dir <<std::string("fluka_newlosses_")<< npart << "_" << seed << std::string(".txt");	 
	//~ ofstream* fluka_output1 = new ofstream(fluka_file.str().c_str());   
	//~ if(!fluka_output1->good()){ std::cerr << "Could not open fluka loss file" << std::endl; exit(EXIT_FAILURE); }  
	//~ myFlukaLosses->Finalise();
	//~ myFlukaLosses->Output(fluka_output1);
	//~ delete fluka_output1;

   /*********************************************************************
	** OUTPUT LOSSMAP  
	*********************************************************************/
	ostringstream dustbin_file;
	dustbin_file << dustbin_dir<<"Dustbin_losses_"<< npart << "_" << seed << std::string(".txt");	
	ofstream* dustbin_output = new ofstream(dustbin_file.str().c_str());	
	if(!dustbin_output->good())    {
        std::cerr << "Could not open dustbin loss file" << std::endl;
        exit(EXIT_FAILURE);
    }   
	
	myLossMapDustbin->Finalise(); 
	myLossMapDustbin->Output(dustbin_output); 
   
    cout << "npart: " << npart << endl;
    cout << "left: " << myBunch->size() << endl;
    cout << "absorbed: " << npart - myBunch->size() << endl;

    delete myMaterialDatabase;
    delete myBunch;
    delete myTwiss;
    delete myAccModel;
    delete myMADinterface;

    return 0;
}
