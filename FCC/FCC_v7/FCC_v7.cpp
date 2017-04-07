//Include relevant C++ headers

#include <iostream> // input/output
#include <sstream> // handles string streams
#include <string>
#include <map>
#include <set>
#include <ctime> // used for random seed
#include <sys/stat.h> //to use mkdir

//include relevant MERLIN headers
#include "AcceleratorModel/Apertures/CollimatorAperture.h"
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
    int npart = 1;				// number of particles to track
    int nturns = 1;				// number of turns to track
	bool DoTwiss = 1;				// run twiss and align to beam envelope etc?
	 
    if (argc >=2){npart = atoi(argv[1]);}
    if (argc >=3){seed = atoi(argv[2]);}

    RandomNG::init(iseed);

    double beam_energy = 50000.0;

    cout << "npart=" << npart << " nturns=" << nturns << " beam energy = " << beam_energy << endl;

	string start_element = "IPA";// IPA
	string tas_element = "TAS.RA";// TAS IPA Right
	string ipb_element = "IPB";// IPB
	string tcld8_element = "TCLD.8RA.H1";
	string tcld10_element = "TCLD.10RA.H1";
	//~ string end_element = "END";// END
	string end_element = ipb_element;// END

    // Define useful variables
    double beam_charge = 1.1e11;
    double normalized_emittance = 3.5e-6;
    double gamma = beam_energy/PhysicalConstants::ProtonMassMeV/PhysicalUnits::MeV;
	double beta = sqrt(1.0-(1.0/pow(gamma,2)));
	double emittance = normalized_emittance/(gamma*beta);
	
	string directory, input_dir, output_dir, pn_dir, case_dir, bunch_dir, lattice_dir, fluka_dir, dustbin_dir;			
			
	//~ string directory = "/afs/cern.ch/user/h/harafiqu/public/MERLIN";					//lxplus harafiqu
	//~ string directory = "/home/haroon/git/Merlin/HR";									//iiaa1	
	directory 	= "/home/HR/Downloads/MERLIN_HRThesis/MERLIN";							//HR MAC		
	input_dir 	= "/FCC/Input/";
	output_dir 	= "/Build/FCC/outputs/FCC_v7/";

		
	string batch_directory="5_APR_Transport/";
	 
	string full_output_dir = (directory+output_dir);
	mkdir(full_output_dir.c_str(), S_IRWXU);
	full_output_dir = (directory+output_dir+batch_directory);
	mkdir(full_output_dir.c_str(), S_IRWXU);
	fluka_dir = full_output_dir + "Fluka/";    
	mkdir(fluka_dir.c_str(), S_IRWXU); 
	
	bool every_bunch			= 0;		// output whole bunch every turn in a single file
	bool rf_test				= 0;		// Use RF distribution to plot RF bucket
		
	bool output_initial_bunch 	= 1;
	bool output_final_bunch 	= 1;
	bool input_distn			= 0;
		if (output_initial_bunch || output_final_bunch || every_bunch){
			bunch_dir = (full_output_dir+"Bunch_Distn/"); 	mkdir(bunch_dir.c_str(), S_IRWXU); 		
		}	
	
	bool output_fluka_database 	= 1;
	bool output_twiss			= 1;		
		if(output_twiss){ lattice_dir = (full_output_dir+"LatticeFunctions/"); mkdir(lattice_dir.c_str(), S_IRWXU); }	
	
	bool collimation_on 		= 1;
		if(collimation_on){
			dustbin_dir = full_output_dir + "LossMap/"; 	mkdir(dustbin_dir.c_str(), S_IRWXU);		
		}		
	bool use_sixtrack_like_scattering = 0;
	bool scatterplot			= 0;
	bool jawinelastic			= 0;
	bool jawimpact				= 0;
	
	bool ap_survey				= 1;
	bool coll_survey			= 1;
	bool output_particletracks	= 0;
	
	bool symplectic				= 1;
	bool sixD					= 0;	//0 = No RF, 1 = Rf	
	bool composite				= 1;	//0 = Sixtrack composite, 1=MERLIN composite	
	bool crossing				= 0;
	bool run_with_twiss			= 1;

	
/************************************
*	ACCELERATORMODEL CONSTRUCTION	*
************************************/
	cout << "MADInterface" << endl;

	MADInterface* myMADinterface;
	if(crossing){
		myMADinterface = new MADInterface( directory+input_dir+"FCC_Lattice_v7_DS_0300_NoCrossing.tfs", beam_energy );		
	}
	else{
		myMADinterface = new MADInterface( directory+input_dir+"FCC_Lattice_v7_DS_0300_NoCrossing.tfs", beam_energy );
	}
		//~ myMADinterface->SetSingleCellRF(1);
	cout << "MADInterface Done" << endl;

    myMADinterface->TreatTypeAsDrift("RFCAVITY");
    //~ myMADinterface->TreatTypeAsDrift("HKICKER");
    //~ myMADinterface->TreatTypeAsDrift("VKICKER");
    //~ myMADinterface->TreatTypeAsDrift("SEXTUPOLE");
    //~ myMADinterface->TreatTypeAsDrift("OCTUPOLE");

    myMADinterface->ConstructApertures(false);

    AcceleratorModel* myAccModel = myMADinterface->ConstructModel();    

	//~ std::vector<RFStructure*> RFCavities;
	//~ myAccModel->ExtractTypedElements(RFCavities,"ACS*");
	//~ Klystron* Kly1 = new Klystron("KLY1",RFCavities);
	//~ Kly1->SetVoltage(0.0);
	//~ Kly1->SetPhase(pi/2);                                                                

/************
*	TWISS	*
************/

    int start_element_number = myAccModel->FindElementLatticePosition(start_element.c_str());
    int tas_element_number = myAccModel->FindElementLatticePosition(tas_element.c_str());
    int end_element_number = myAccModel->FindElementLatticePosition(ipb_element.c_str());
    int tcld8_element_number = myAccModel->FindElementLatticePosition(tcld8_element.c_str());
    int tcld10_element_number = myAccModel->FindElementLatticePosition(tcld10_element.c_str());
    
    //~ std::vector<AcceleratorComponent*> elements;
    //~ myAccModel->ExtractTypedElements(elements,"*");
    //~ int end_element_number = elements.size();
    
    cout << "Found start element "<< start_element << " at element number " << start_element_number << endl;
    cout << "Found "<< tas_element << " at element number " << tas_element_number << endl;
    cout << "Found lattice end "<< end_element << " at element number " << end_element_number << endl;

	LatticeFunctionTable* myTwiss = new LatticeFunctionTable(myAccModel, beam_energy);
	Dispersion* myDispersion = new Dispersion(myAccModel, beam_energy);
	
	if (run_with_twiss){		
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
	}
	//~ if(sixD)
	//~ Kly1->SetVoltage(2.0);


/************************
*	Collimator set up	*
************************/
/*
	cout << "Collimator Setup" << endl;   
   
	MaterialDatabase* myMaterialDatabase = new MaterialDatabase();	
    CollimatorDatabase* collimator_db;
	
	collimator_db = new CollimatorDatabase( directory+input_dir+"Collimator_pure.txt", myMaterialDatabase,  true);
	
    collimator_db->MatchBeamEnvelope(false);
    collimator_db->EnableJawAlignmentErrors(false);
    collimator_db->UseMiddleJawHalfGap();

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
    */
/****************************
*	Aperture Configuration	*
****************************/

	ApertureConfiguration* myApertureConfiguration;
	if(crossing){
		myApertureConfiguration = new ApertureConfiguration(directory+input_dir+"FCC_v7_DS_0300_Aperture.tfs",0);    	
	}
	else{
		myApertureConfiguration = new ApertureConfiguration(directory+input_dir+"FCC_v7_DS_0300_Aperture.tfs",0);     
	}
    	
	//~ ostringstream ap_output_file;
	//~ ap_output_file << full_output_dir << "ApertureConfiguration.log";
	//~ ofstream* ApertureConfigurationLog = new ofstream(ap_output_file.str().c_str());
    //~ myApertureConfiguration->SetLogFile(*ApertureConfigurationLog);
	//~ myApertureConfiguration->EnableLogging(true);
	
    myApertureConfiguration->ConfigureElementApertures(myAccModel);
    delete myApertureConfiguration;
    
	if(ap_survey){
		ApertureSurvey* myApertureSurvey = new ApertureSurvey(myAccModel, full_output_dir, 0.1, 0);
		//~ ApertureSurvey* myApertureSurvey = new ApertureSurvey(myAccModel, full_output_dir, 0, 20);
	}
		
	if(coll_survey){
		CollimatorSurvey* CollSurvey = new CollimatorSurvey(myAccModel, emittance, emittance, myTwiss); 
		ostringstream cs_output_file;
		cs_output_file << full_output_dir << "coll_survey.txt";
		ofstream* cs_output = new ofstream(cs_output_file.str().c_str());
		if(!cs_output->good()) { std::cerr << "Could not open collimator survey output" << std::endl; exit(EXIT_FAILURE); }   
		CollSurvey->Output(cs_output, 100);			
		delete cs_output;
	}

/********************
*	Beam Settings	*
********************/
	
	BeamData mybeam;

	// Default values for all members of BeamData are 0.0
	// Particles are treated as macro particles - this has no bearing on collimation
	mybeam.charge = beam_charge/npart;
	mybeam.p0 = beam_energy;
	if (run_with_twiss){
		mybeam.beta_x = myTwiss->Value(1,1,1,start_element_number)*meter;
		mybeam.beta_y = myTwiss->Value(3,3,2,start_element_number)*meter;
		mybeam.alpha_x = -myTwiss->Value(1,2,1,start_element_number);
		mybeam.alpha_y = -myTwiss->Value(3,4,2,start_element_number);
		
		// Minimum and maximum sigma for HEL Halo Distribution
		//~ mybeam.min_sig_x = 6;
		//~ mybeam.max_sig_x = 6.00156;
		//~ mybeam.min_sig_y = 0.0;
		//~ mybeam.max_sig_y = 0.0;    
	   
		// Dispersion
		mybeam.Dx=myDispersion->Dx;
		mybeam.Dy=myDispersion->Dy;
		mybeam.Dxp=myDispersion->Dxp;
		mybeam.Dyp=myDispersion->Dyp;
		
		//Beam centroid
		mybeam.x0=myTwiss->Value(1,0,0,start_element_number);
		mybeam.xp0=myTwiss->Value(2,0,0,start_element_number);
		mybeam.y0=myTwiss->Value(3,0,0,start_element_number);
		mybeam.yp0=myTwiss->Value(4,0,0,start_element_number);
		mybeam.ct0=myTwiss->Value(5,0,0,start_element_number);
		
		delete myDispersion;
	}

		mybeam.emit_x = emittance * meter;
		mybeam.emit_y = emittance * meter;


		//~ mybeam.sig_dp = 0.0;
		mybeam.sig_z = 0.0;
		//~ if(rf_test){
			//~ mybeam.sig_z = ((2.51840894498383E-10 * 299792458)) * meter;
		//~ }
		//~ mybeam.sig_dp =  (1.129E-4);

		// X-Y coupling
		mybeam.c_xy=0.0;
		mybeam.c_xyp=0.0;
		mybeam.c_xpy=0.0;
		mybeam.c_xpyp=0.0;

			
/************************
*	TCLD Settings		*
************************/

	// Extract TCLDs
	vector<Collimator*> TCLD8;
    myAccModel->ExtractTypedElements(TCLD8, tcld8_element.c_str()); 
    
	vector<Collimator*> TCLD10;
    myAccModel->ExtractTypedElements(TCLD10, tcld10_element.c_str()); 
    
    
	// Create Collimator Aperture
	// CollimatorAperture* app=new CollimatorAperture(CollData[n].x_gap,CollData[n].y_gap,CollData[n].tilt,collimator_material, (CMapit->second)->GetLength(), 0,0);

	 //~ CollimatorAperture* app8=new CollimatorAperture(

	//~ TCLD8->SetAperture(app8);


		
		
/************
*	BUNCH	*
************/

    ProtonBunch* myBunch;
    int node_particles = npart;
    ParticleBunchConstructor* myBunchCtor;
    
    if(input_distn){
		ifstream* bunch_input = new ifstream("/home/HR/Downloads/MERLIN_HRThesis/MERLIN/FCC/Input/initial_merlin_distn.50tev.500k.3m.noX.all.dat");	
		//~ ifstream* bunch_input = new ifstream("/home/HR/Downloads/MERLIN_HRThesis/MERLIN/FCC/Input/initial_merlin_distn.50tev.500k.3m.noX.inelastic.dat");	
		istream* is = bunch_input;		
		myBunch = new ProtonBunch(beam_energy, mybeam.charge, *is);					
		myBunch->SetMacroParticleCharge(mybeam.charge);
		npart = myBunch->size();
		node_particles = npart;
		cout << "\n\n\tInput distribution of " << npart << " particles used to create bunch" << endl;		
	}
	else{
		if(rf_test){
			//~ myBunchCtor = new ParticleBunchConstructor(mybeam, node_particles, RFDistn);
			myBunchCtor = new ParticleBunchConstructor(mybeam, node_particles, RFDistn2);
		}
		else{
			//~ myBunchCtor = new ParticleBunchConstructor(mybeam, node_particles, Halo7TeV);
			//~ myBunchCtor = new ParticleBunchConstructor(mybeam, node_particles, normalDistribution);
			myBunchCtor = new ParticleBunchConstructor(mybeam, node_particles, pencilDistribution);
		}
		
		myBunch = myBunchCtor->ConstructParticleBunch<ProtonBunch>();
		delete myBunchCtor;

		myBunch->SetMacroParticleCharge(mybeam.charge);
	}
    
    if(output_initial_bunch){   
		ostringstream hbunch_output_file;
		//~ hbunch_output_file << bunch_dir << seed  << "_initial_bunch.txt";
		hbunch_output_file << bunch_dir << "initial_bunch.txt";
		ofstream* hbunch_output = new ofstream(hbunch_output_file.str().c_str());
		if(!hbunch_output->good()) { std::cerr << "Could not open initial bunch output" << std::endl; exit(EXIT_FAILURE); }   
		myBunch->Output(*hbunch_output);			
		delete hbunch_output;	
	} 

/************************
*	ParticleTracker		*
************************/


	ParticleTracker* myParticleTracker1;
    ParticleTracker* myParticleTracker2;

	AcceleratorModel::Beamline beamline1 = myAccModel->GetBeamline(start_element_number, tas_element_number-2);
	//~ AcceleratorModel::Beamline beamline2 = myAccModel->GetBeamline(tas_element_number, end_element_number);
	AcceleratorModel::Beamline beamline2 = myAccModel->GetBeamline(tas_element_number-1, end_element_number-1);
	
	myParticleTracker1 = new ParticleTracker(beamline1, myBunch);
	myParticleTracker2 = new ParticleTracker(beamline2, myBunch);

	if(symplectic){
		myParticleTracker1->SetIntegratorSet(new ParticleTracking::SYMPLECTIC::StdISet());	
		myParticleTracker2->SetIntegratorSet(new ParticleTracking::SYMPLECTIC::StdISet());
	}
	else{	
		myParticleTracker1->SetIntegratorSet(new ParticleTracking::TRANSPORT::StdISet());	
		myParticleTracker2->SetIntegratorSet(new ParticleTracking::TRANSPORT::StdISet());	
	}


	//~ AcceleratorModel::RingIterator beamline = myAccModel->GetRing(start_element_number);
    //~ ParticleTracker* myParticleTracker = new ParticleTracker(beamline, myBunch);
    //~ if(symplectic)
		//~ myParticleTracker->SetIntegratorSet(new ParticleTracking::SYMPLECTIC::StdISet());
    //~ else
		//~ myParticleTracker->SetIntegratorSet(new ParticleTracking::TRANSPORT::StdISet());
    
	string tof = "Tracking_output_file.txt";
    string t_o_f = full_output_dir+tof;
    
	if(output_particletracks){
		ostringstream trackingparticles_sstream;
		//~ trackingparticles_sstream << full_output_dir << "Tracking_output_file_"<< npart << "_" << seed << std::string(".txt");     
		trackingparticles_sstream << full_output_dir << "Tracking_output_file_"<< npart << std::string(".txt");     
		string trackingparticles_file = trackingparticles_sstream.str().c_str();     
		 
		TrackingOutputAV* myTrackingOutputAV = new TrackingOutputAV(trackingparticles_file);
		myTrackingOutputAV->SetSRange(0, 100000);
		myTrackingOutputAV->SetTurn(1);
		myTrackingOutputAV->output_all = 1;
		 
		myParticleTracker1->SetOutput(myTrackingOutputAV);
	}
	
/****************************
*	Collimation Process		*
****************************/

	//~ FlukaLosses* myFlukaLosses = new FlukaLosses;  
	LossMapDustbin* myLossMapDustbin = new LossMapDustbin(nearestelement);
	//~ LossMapDustbin* myLossMapDustbin = new LossMapDustbin();
	ScatteringModel* myScatter = new ScatteringModel;

  	if(collimation_on){ 
		
		bool beam2 = 0;
		
		CollimateProtonProcess* myCollimateProcess =new CollimateProtonProcess(2, 4, NULL);
		
		//~ myLossMapDustbin->Beam2(beam2);
		myCollimateProcess->SetDustbin(myLossMapDustbin);   	        
		//~ myCollimateProcess->SetFlukaLosses(myFlukaLosses); 
		
		//~ myCollimateProcess->ScatterAtCollimator(true);
		myCollimateProcess->ScatterAtCollimator(false);
		
		// Extract TAS and set collimator aperture
	    //~ std::vector<Collimator*> taselements;
	    //~ std::vector<Collimator*>::iterator tasit;
		//~ myAccModel->ExtractTypedElements(taselements,"TAS.RA");
		//~ int tas_test = taselements.size();
		//~ tasit = taselements.begin();
		
		//~ if(tas_test == 1){
			//~ cout << "1 TAS.RA found" << endl;
			
			//~ // Set TAS to Copper
			//~ MaterialDatabase* myMaterialDatabase = new MaterialDatabase();
			//~ Material* collimator_material = myMaterialDatabase->FindMaterial("Cu");
			
			//~ //create aperture
			//~ // CircularCollimatorAperture(radius, tilt, Material, double length, double x_offset_entry=0.0, double y_offset_entry=0.0);
			//~ CircularCollimatorAperture* tas_ap = new CircularCollimatorAperture(0.025, 0.0, collimator_material, 3.0, 0,0);
			
			//~ //set aperture
			
			//~ (*tasit)->SetAperture(tas_ap);
		//~ }
		//~ else{
		//~ cout << "Number of TAS.RA found: " << tas_test;			
		//~ }

		if(composite){	myScatter->SetComposites(1);}
		else{			myScatter->SetComposites(0);}

		if(jawimpact){
			myScatter->SetJawImpact("TAS.RA");
			//~ myScatter->SetJawImpact("TCSG.B5L7.B1");
		}
		if(scatterplot)	{
			myScatter->SetScatterPlot("TAS.RA");			
			//~ myScatter->SetScatterPlot("TCSG.B5L7.B1");
		}
		if(jawinelastic){
			myScatter->SetJawInelastic("TAS.RA");
			//~ myScatter->SetJawInelastic("TCSG.B5L7.B1");
		}

		// 0: ST,    1: ST + Adv. Ionisation,    2: ST + Adv. Elastic,   
		// 3: ST + Adv. SD,     4: MERLIN
		// Where ST = SixTrack like, Adv. = Advanced, SD = Single Diffractive,
		// and MERLIN includes all advanced scattering
		if(use_sixtrack_like_scattering){myScatter->SetScatterType(0);}
		else{myScatter->SetScatterType(4);}
		myCollimateProcess->SetScatteringModel(myScatter);
		myCollimateProcess->SetLossThreshold(200.0);
		myCollimateProcess->SetOutputBinSize(0.1);

		myParticleTracker1->AddProcess(myCollimateProcess);
		myParticleTracker2->AddProcess(myCollimateProcess);
	}
	
	
/********************
 *  TRACKING RUN	*
 *******************/
	ostringstream cbo_file;
	cbo_file << bunch_dir << "Every_bunch.txt";
	ofstream* cboclean = new ofstream(cbo_file.str().c_str(), ios::trunc);
	ofstream* cbo = new ofstream(cbo_file.str().c_str(), ios::app);		
	if(!cbo->good())	{ std::cerr << "Could not open every bunch output file" << std::endl; exit(EXIT_FAILURE); }
	
    // Now all we have to do is create a loop for the number of turns and use the Track() function to perform tracking   
	
	for (int turn=1; turn<=nturns; turn++)
    {
        cout << "Turn " << turn <<"\tParticle number: " << myBunch->size() << endl;

        //~ myParticleTracker->Track(myBunch);
        
        myParticleTracker1->Track(myBunch);
        
		//~ if(output_final_bunch){   
			ostringstream tas_output_file;
			tas_output_file << bunch_dir << "TAS_bunch.txt";
			ofstream* tas_output = new ofstream(tas_output_file.str().c_str());
			if(!tas_output->good()) { std::cerr << "Could not open TAS bunch output" << std::endl; exit(EXIT_FAILURE); }   
			myBunch->Output(*tas_output);			
			delete tas_output;	
		//~ }

        myParticleTracker2->Track(myBunch);
        
        if(every_bunch){myBunch->Output(*cbo); }   
        
        if( myBunch->size() <= 1 ) break;
    }
   
	/*********************************************************************
	** OUTPUT FINAL BUNCH
	*********************************************************************/
	if(output_final_bunch){   
		ostringstream bunch_output_file;
		//~ bunch_output_file << bunch_dir << seed  << "_final_bunch.txt";
		//~ bunch_output_file << bunch_dir << "final_bunch.txt";
		bunch_output_file << bunch_dir << "IPB_bunch.txt";
		ofstream* bunch_output = new ofstream(bunch_output_file.str().c_str());
		if(!bunch_output->good()) { std::cerr << "Could not open final bunch output" << std::endl; exit(EXIT_FAILURE); }   
		myBunch->Output(*bunch_output);			
		delete bunch_output;	
	}

	/*********************************************************************
	**	Output Jaw Impact, Scatter Plot, Jaw Inelastic
	*********************************************************************/
	string JawIm_dir = (full_output_dir+"Jaw_Impact/"); 	mkdir(JawIm_dir.c_str(), S_IRWXU); 	
	//~ myScatter->OutputJawImpact(JawIm_dir,seed);
	myScatter->OutputJawImpact(JawIm_dir,0);
	string JawInel_dir = (full_output_dir+"Jaw_Inelastic/"); 	mkdir(JawInel_dir.c_str(), S_IRWXU); 	
	//~ myScatter->OutputJawInelastic(JawInel_dir,seed);	
	myScatter->OutputJawInelastic(JawInel_dir,0);	
	string SPlot_dir = (full_output_dir+"Scatter_Plot/"); 	mkdir(SPlot_dir.c_str(), S_IRWXU); 	
    //~ myScatter->OutputScatterPlot(SPlot_dir,seed);
    myScatter->OutputScatterPlot(SPlot_dir,0);
   
	/*********************************************************************
	** OUTPUT FLUKA LOSSES 
	*********************************************************************/
	//~ ostringstream fluka_dustbin_file;
	//~ fluka_dustbin_file << full_output_dir<<std::string("fluka_losses_")<< npart << "_" << seed << std::string(".txt");	   
	  
	//~ ofstream* fluka_dustbin_output = new ofstream(fluka_dustbin_file.str().c_str());	
	//~ if(!fluka_dustbin_output->good())    {
        //~ std::cerr << "Could not open dustbin loss file" << std::endl;
        //~ exit(EXIT_FAILURE);
    //~ }   
	
	//~ myFlukaDustbin->Finalise(); 
	//~ myFlukaDustbin->Output(fluka_dustbin_output); 
  
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
	//~ dustbin_file << dustbin_dir <<"Dustbin_losses_"<< npart << "_" << seed << std::string(".txt");	
	dustbin_file << dustbin_dir <<"Dustbin_losses_"<< npart << std::string(".txt");	
	ofstream* dustbin_output = new ofstream(dustbin_file.str().c_str());	
	if(!dustbin_output->good())    {
        std::cerr << "Could not open dustbin loss file" << std::endl;
        exit(EXIT_FAILURE);
    }   
	
	myLossMapDustbin->Finalise(); 
	myLossMapDustbin->Output(dustbin_output); 
   
    // These lines tell us how many particles we tracked, how many survived, and how many were lost
    cout << "npart: " << npart << endl;
    cout << "left: " << myBunch->size() << endl;
    cout << "absorbed: " << npart - myBunch->size() << endl;

    // Cleanup our pointers on the stack for completeness
    //~ delete myMaterialDatabase;
    delete myBunch;
    delete myTwiss;
    delete myAccModel;
    delete myMADinterface;

    return 0;
}
