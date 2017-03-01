
///* \brief An implementation of a Poisson type neuron, implements INeuron class. The presynaptic rate activity is set and a method setFireRate
/// is provided to change it  during runtime. note Averaging would introduce complexities during testing.
/// The change in rate should be done externally and set to newrate =Pre Rate*Synaptic strength
/// As every pre neuron is assumed to have the same rate, the update occurs in ActionPotentialEvent
/// and is mLastFireRate*totalSynapticStrength
/// \author Kostantinos Lagogiannis 2007

#include "stdafx.h"
#include "synapseSW.h"
#include "synapseEnsemble.h"
#include "synapticTransmission.h"
#include "INeuron.h"
#include "PoissonNeuron.h"

PoissonNeuron::PoissonNeuron(float timestep,short ID,int StartFireRate,bool FixedRate)
{
    bPostspiketoLog     = false;
	h					= timestep;	
	mID					= ID;
	iLastSynapseIndex	= 0;
	msNumberOfSpikesInPeriod = 0;
	mfPeriodOfSpikeCount	= 0.0f;
    //msLastFireRate			= StartFireRate;
    mlamda                  = StartFireRate;
	mbFixedRate				= FixedRate;

	//Seed random number generator
	 rng_r = gsl_rng_alloc (gsl_rng_mt19937);
	 if (!rng_r) throw "GSL RNG Init Failed. Out Of Memory?";

//	 unsigned int seed = unsigned(time(&t)) + rand()*100;
	 unsigned int seed = rand()*100;

	 gsl_rng_set(rng_r,seed);

	 //Seed random number generator
	 srand(seed); 

	//srand(unsigned(time(&t)));
}

//adds Afferent to array and registers target neuron with it
//Same As IFNeuron
/// \name RegisterAfferent
/// \brief It connects to an afferent neuron via the ensemble Synapse
/// \param A synapse Ensemble/set connecing the Afferent neuron to this neuron
void PoissonNeuron::RegisterAfferent(ISynapseEnsemble* pSyn)
{
	//Check that the array is not Maxed.
	if (iLastSynapseIndex == (MAX_AFFERENTS-1))
	{
		std::cout << "Max afferent number reached, Increase MAX_AFFERENTS ";
		abort();
		throw "Max afferent number reached, Increase MAX_AFFERENTS ";
        /// \todo Can have code to increase array size with malloc() or convert to std vector

	}

	//Register Back to the Synapse Ensemble
	pSyn->RegisterNeuron(this);
	//Add to private array
	mSynapses[iLastSynapseIndex] = pSyn;
	iLastSynapseIndex++;

}

void PoissonNeuron::SpikeArrived(synapticTransmission* s)
{

	//Clear memory from new object
	delete s; 
}
//Called on every Simulation step
void PoissonNeuron::StepSimTime()
{
    float noise =0.0; // PoissonSource::randGauss(0,4.0f*sigma,sigma,2.0f*sigma);
    bPostspiketoLog =false;
	//poisson spike draw
    //double r = (rand()/(double)RAND_MAX);
    double r = gsl_rng_uniform(rng_r);
    //if ((msLastFireRate*h) > r)

    mfPeriodOfSpikeCount+=h;

    if ((mlamda*h + noise)> r)
    {
         ActionPotentialEvent();
         msNumberOfSpikesInPeriod++;
    }

}

/// \returns true is neuron generated spike in the last time step
bool  PoissonNeuron::ActionPotentialOccured()
{
    return bPostspiketoLog;
}

/// \brief Called when neuron Produces output
void PoissonNeuron::ActionPotentialEvent() 
{
	double nRate= 0;
    for(unsigned int i=0;i<iLastSynapseIndex;i++)
	{
		if (!mSynapses[i]) break; //Null so Next
		//Notify all synapses (Time steps Fwd?)
        mSynapses[i]->SpikeArrived(ISynapse::SPIKE_POST);
		
        ///Deprecated nRate+= mSynapses[i]->getSourceFireRate()*mSynapses[i]->getAvgStrength();
	}

    ////Removed Assume all pre neurons have the same strength
    ///if (!mbFixedRate) setFireRate(nRate);
    ///
    bPostspiketoLog     = true;
	//Simple Linear Relation As in STDP Paper
	//setFireRate(20);
#ifdef VERBOSE
		std::cout << std::endl << "-* Neuron POST Spike new Rate: " << nRate;
#endif

}

/// \brief Fire Rate Reset to new Value.
/// The change in rate should be done externally and set to
/// newrate =Pre Rate*Synaptic strength
void PoissonNeuron::setFireRate(float newFireRate)
{
    //msLastFireRate = newFireRate;
    mlamda         = newFireRate;
}

/// \brief Returns actual cound using the number of Spikes In the previous Elapsed second
float PoissonNeuron::getFireRate()
{
    msLastFireRate = msNumberOfSpikesInPeriod/mfPeriodOfSpikeCount;

    msNumberOfSpikesInPeriod=0; //Reset Counter
    mfPeriodOfSpikeCount = 0;
    return msLastFireRate;

    //return mlamda;

}


int PoissonNeuron::getID()
{
	return mID;
}

PoissonNeuron::~PoissonNeuron(void)
{
}
