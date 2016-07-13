#include <fstream>

#include "AcceleratorModel/ApertureSurvey.h"
#include "AcceleratorModel/StdComponent/Collimator.h"
#include "BeamDynamics/ParticleTracking/ParticleBunch.h"

using namespace std;

static ofstream* open_file(const string file_name)
{
	ofstream* output_file = new ofstream(file_name.c_str());
	if(!output_file->good())
	{
		std::cerr << "Could not open ApertureSurvey file:" << file_name << std::endl;
		exit(EXIT_FAILURE);
	}
	return output_file;
}

void ApertureSurvey::ApertureSurvey(AcceleratorModel* model, string file_name, double step_size, size_t points_per_element)
{
	ofstream* output_file = open_file(file_name);
	ApertureSurvey(model,output_file, step_size, points_per_element);
	delete output_file;
}

void ApertureSurvey::ApertureSurvey(AcceleratorModel* model, string file_name, bool exact_s, double step_size)
{
	ofstream* output_file = open_file(file_name);
	ApertureSurvey(model, output_file, exact_s, step_size);
	delete output_file;
}

void ApertureSurvey::ApertureSurvey(AcceleratorModel* model, std::ostream* os, double step_size, size_t points_per_element)
{
	double points = points_per_element;

	(*os) << "#name\ttype\ts_end\tlength\tap_px\tap_mx\tap_py\tap_my" << endl;

	//~ DoApertureSurvey();
	double s = 0;
	double last_sample = 0-step_size;
	double lims[4];
	//~ cout << "aperture_survey" << endl;

	for (AcceleratorModel::BeamlineIterator bi = model->GetBeamline().begin(); bi != model->GetBeamline().end(); bi++)
	{

		AcceleratorComponent *ac = &(*bi)->GetComponent();

		if (fabs(s - ac->GetComponentLatticePosition())> 1e-6)
		{
			exit(1);
		}

		Aperture* ap =	ac->GetAperture();
		Collimator* aCollimator = dynamic_cast<Collimator*>(ac);

		//cout << "ap "<< s << " " << ac->GetName() << " " << ac->GetLength() << ((aCollimator!=NULL)?" Collimator":"");
		//if (ap != NULL) cout << " " << ap->GetMaterial();
		//cout << endl;

		vector<double> zs;
		if (points > 0)
		{
			for (size_t i=0; i<points; i++)
			{
				zs.push_back(i*ac->GetLength()*(1.0/(points-1)));
			}
		}
		else
		{
			//~ cout << "last_sample = " << last_sample <<endl;
			while (last_sample+step_size < s + ac->GetLength())
			{
				last_sample += step_size;
				//~ //cout << "add step " <<  last_sample << endl;
				zs.push_back(last_sample-s);
			}
		}
		for (size_t zi = 0; zi < zs.size(); zi++)
		{
			double z = zs[zi];
			//~ cout << "call check_aperture(" << z+s << ")" << endl;
			if (ap != NULL)
			{
				ApertureSurvey::CheckAperture(ap, z, lims);
			}
			else
			{
				lims[0] = lims[1] = lims[2]= lims[3] = 1;
			}

			(*os) << ac->GetName() << "\t";
			(*os) << ac->GetType() << "\t";
			(*os) << ac->GetComponentLatticePosition()+ac->GetLength() << "\t";
			(*os) << ac->GetLength() << "\t";
			//~ (*os) << s+z << "\t";
			(*os) << lims[0] << "\t";
			(*os) << lims[1] << "\t";
			(*os) << lims[2] << "\t";
			(*os) << lims[3] << endl;
		}
		s += ac->GetLength();
	}
}

void ApertureSurvey::ApertureSurvey(AcceleratorModel* model, std::ostream* os, bool exact_s, double step_size)
{
	if(!exact_s)
	{
		ApertureSurvey(model, os, step_size);
	}
	else
	{
		double points = 0;

		(*os) << "#name\ttype\ts_end\tlength\tap_px\tap_mx\tap_py\tap_my" << endl;

		//~ DoApertureSurvey();
		double s = 0;
		double last_sample = 0-step_size;
		double lims[4];
		//~ cout << "aperture_survey" << endl;

		for (AcceleratorModel::BeamlineIterator bi = model->GetBeamline().begin(); bi != model->GetBeamline().end(); bi++)
		{

			AcceleratorComponent *ac = &(*bi)->GetComponent();

			if (fabs(s - ac->GetComponentLatticePosition())> 1e-6)
			{
				exit(1);
			}

			Aperture* ap =	ac->GetAperture();
			Collimator* aCollimator = dynamic_cast<Collimator*>(ac);

			//cout << "ap "<< s << " " << ac->GetName() << " " << ac->GetLength() << ((aCollimator!=NULL)?" Collimator":"");
			//if (ap != NULL) cout << " " << ap->GetMaterial();
			//cout << endl;

			vector<double> zs;
			if (points > 0)
			{
				for (size_t i=0; i<points; i++)
				{
					zs.push_back(i*ac->GetLength()*(1.0/(points-1)));
				}
			}
			else
			{
				//~ cout << "last_sample = " << last_sample <<endl;
				while (last_sample+step_size < s + ac->GetLength())
				{
					last_sample += step_size;
					//~ //cout << "add step " <<  last_sample << endl;
					zs.push_back(last_sample-s);
				}
			}
			for (size_t zi = 0; zi < zs.size(); zi++)
			{
				double z = zs[zi];
				//~ cout << "call check_aperture(" << z+s << ")" << endl;
				if (ap != NULL)
				{
					ApertureSurvey::CheckAperture(ap, z, lims);
				}
				else
				{
					lims[0] = lims[1] = lims[2]= lims[3] = 1;
				}

				(*os) << ac->GetName() << "\t";
				(*os) << ac->GetType() << "\t";
				(*os) << ac->GetComponentLatticePosition()+z << "\t";
				//~ (*os) << ac->GetComponentLatticePosition()+ac->GetLength() << "\t";
				(*os) << ac->GetLength() << "\t";
				(*os) << lims[0] << "\t";
				(*os) << lims[1] << "\t";
				(*os) << lims[2] << "\t";
				(*os) << lims[3] << endl;
			}
			s += ac->GetLength();
		}
	}
}

void ApertureSurvey::CheckAperture(Aperture* ap, double s, double *aps)
{
	//~ cout << "CheckAperture" << endl;
	const double step = 1e-6;
	const double max = 1.0;
	const double min = 0.0;

	// iterate through directions
	for (int dir=0; dir<4; dir++)
	{
		double xdir=0, ydir=0;
		if (dir==0)
		{
			xdir=+1;
		}
		else if (dir==1)
		{
			xdir=-1;
		}
		else if (dir==2)
		{
			ydir=+1;
		}
		else if (dir==3)
		{
			ydir=-1;
		}

		aps[dir] = 0;

		// scan for limit
		double below=min, above=max;

		while(above-below > step)
		{
			double guess = (above+below)/2;

			if (ap->PointInside(xdir*guess, ydir*guess, s))
			{
				below = guess;
			}
			else
			{
				above = guess;
			}
		}
		aps[dir] = (above+below)/2;
	}
}
