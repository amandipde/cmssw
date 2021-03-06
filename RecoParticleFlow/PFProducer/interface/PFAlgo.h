#ifndef RecoParticleFlow_PFProducer_PFAlgo_h
#define RecoParticleFlow_PFProducer_PFAlgo_h 

#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/Common/interface/OrphanHandle.h"
#include "DataFormats/MuonReco/interface/MuonFwd.h"
#include "DataFormats/ParticleFlowReco/interface/PFBlockFwd.h"
#include "DataFormats/ParticleFlowReco/interface/PFBlock.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateFwd.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateElectronExtra.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateElectronExtraFwd.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidatePhotonExtra.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidatePhotonExtraFwd.h"
#include "DataFormats/ParticleFlowReco/interface/PFBlockElement.h"
#include "DataFormats/ParticleFlowReco/interface/PFRecHitFwd.h"
#include "DataFormats/ParticleFlowReco/interface/PFClusterFwd.h"
#include "DataFormats/ParticleFlowReco/interface/PFRecTrackFwd.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectronFwd.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "RecoParticleFlow/PFProducer/interface/PFCandConnector.h"
#include "RecoParticleFlow/PFProducer/interface/PFMuonAlgo.h"
#include "RecoParticleFlow/PFProducer/interface/PFEGammaFilters.h"

#include <iostream>

/// \brief Particle Flow Algorithm
/*!
  \author Colin Bernet
  \date January 2006
*/


class PFEnergyCalibration;
class PFSCEnergyCalibration;
class PFEnergyCalibrationHF;
class PFMuonAlgo;

class PFAlgo {

 public:

  /// constructor
  PFAlgo(bool debug);

  void setHOTag(bool ho) { useHO_ = ho;}
  void setPFMuonAlgo(PFMuonAlgo* algo) {pfmu_ =algo;}
  void setMuonHandle(const edm::Handle<reco::MuonCollection>&);

  void setParameters(double nSigmaECAL,
                     double nSigmaHCAL, 
                     const std::shared_ptr<PFEnergyCalibration>& calibration,
		     const std::shared_ptr<PFEnergyCalibrationHF>& thepfEnergyCalibrationHF);
  
  void setCandConnectorParameters( const edm::ParameterSet& iCfgCandConnector ){
    connector_.setParameters(iCfgCandConnector);
  }
 
  void setCandConnectorParameters(bool bCorrect, 
				  bool bCalibPrimary, 
				  double dptRel_PrimaryTrack, 
				  double dptRel_MergedTrack, 
				  double ptErrorSecondary, 
				  const std::vector<double>& nuclCalibFactors){
    connector_.setParameters(bCorrect, bCalibPrimary, dptRel_PrimaryTrack, dptRel_MergedTrack, ptErrorSecondary, nuclCalibFactors);
  }


  void setPFMuonAndFakeParameters(const edm::ParameterSet& pset);

  void setBadHcalTrackParams(const edm::ParameterSet& pset);
   
  PFMuonAlgo*  getPFMuonAlgo();
  
  void setEGammaParameters(bool use_EGammaFilters, bool useProtectionsForJetMET);

  
  void setEGammaCollections(const edm::View<reco::PFCandidate> & pfEgammaCandidates,
			    const edm::ValueMap<reco::GsfElectronRef> & valueMapGedElectrons, 
 			    const edm::ValueMap<reco::PhotonRef> & valueMapGedPhotons); 
  
  void setPostHFCleaningParameters(bool postHFCleaning,
				   double minHFCleaningPt,
				   double minSignificance,
				   double maxSignificance,
				   double minSignificanceReduction,
				   double maxDeltaPhiPt,
				   double minDeltaMet);

  void setDisplacedVerticesParameters(bool rejectTracks_Bad,
				      bool rejectTracks_Step45,
				      bool usePFNuclearInteractions,
				      bool usePFConversions,
				      bool usePFDecays,
				      double dptRel_DispVtx);
  
  //MIKEB : Parameters for the vertices..
  void setPFVertexParameters(bool useVertex, reco::VertexCollection const&  primaryVertices);
  
  // FlorianB : Collection of e/g electrons
  void setEGElectronCollection(const reco::GsfElectronCollection & egelectrons);

  /// reconstruct particles 
  void reconstructParticles( const reco::PFBlockHandle& blockHandle, PFEGammaFilters const* pfegamma );

  /// Check HF Cleaning
  void checkCleaning( const reco::PFRecHitCollection& cleanedHF );

  /// \return collection of cleaned HF candidates
  std::unique_ptr<reco::PFCandidateCollection> transferCleanedCandidates() {
    return std::move(pfCleanedCandidates_);
  }
  
  /// \return the collection of candidates
  reco::PFCandidateCollection transferCandidates() {
    return connector_.connect(*pfCandidates_);
  }
  
  /// return the pointer to the calibration function
  PFEnergyCalibration* thePFEnergyCalibration() { 
    return calibration_.get();
  }

  friend std::ostream& operator<<(std::ostream& out, const PFAlgo& algo);
  
 private:

  /// process one block. can be reimplemented in more sophisticated 
  /// algorithms
  void processBlock( const reco::PFBlockRef& blockref,
                             std::list<reco::PFBlockRef>& hcalBlockRefs, 
                             std::list<reco::PFBlockRef>& ecalBlockRefs, PFEGammaFilters const* pfegamma );
  
  /// Reconstruct a charged particle from a track
  /// Returns the index of the newly created candidate in pfCandidates_
  /// Michalis added a flag here to treat muons inside jets
  unsigned reconstructTrack( const reco::PFBlockElement& elt,bool allowLoose= false);

  /// Reconstruct a neutral particle from a cluster. 
  /// If chargedEnergy is specified, the neutral 
  /// particle is created only if the cluster energy is significantly 
  /// larger than the chargedEnergy. In this case, the energy of the 
  /// neutral particle is cluster energy - chargedEnergy

  unsigned reconstructCluster( const reco::PFCluster& cluster,
                               double particleEnergy,
			       bool useDirection = false,
			       double particleX=0.,
			       double particleY=0.,
			       double particleZ=0.);

  void setHcalDepthInfo(reco::PFCandidate & cand, const reco::PFCluster& cluster) const ;

  /// todo: use PFClusterTools for this
  double neutralHadronEnergyResolution( double clusterEnergy,
					double clusterEta ) const;

 
  double nSigmaHCAL( double clusterEnergy, 
		     double clusterEta ) const;

  std::unique_ptr<reco::PFCandidateCollection>    pfCandidates_;
  // the post-HF-cleaned candidates
  std::unique_ptr<reco::PFCandidateCollection>    pfCleanedCandidates_;

  /// Associate PS clusters to a given ECAL cluster, and return their energy
  void associatePSClusters(unsigned iEcal,
			   reco::PFBlockElement::Type psElementType,
			   const reco::PFBlock& block,
			   const edm::OwnVector< reco::PFBlockElement >& elements, 
			   const reco::PFBlock::LinkData& linkData, 
			   std::vector<bool>& active, 
			   std::vector<double>& psEne);

  bool isFromSecInt(const reco::PFBlockElement& eTrack,  std::string order) const;


  // Post HF Cleaning
  void postCleaning();

  /// create a reference to a block, transient or persistent 
  /// depending on the needs
  reco::PFBlockRef createBlockRef( const reco::PFBlockCollection& blocks, 
				   unsigned bi );
    
  /// input block handle (full framework case)
  reco::PFBlockHandle    blockHandle_;

  /// number of sigma to judge energy excess in ECAL
  double             nSigmaECAL_;
  
  /// number of sigma to judge energy excess in HCAL
  double             nSigmaHCAL_;
  
  std::shared_ptr<PFEnergyCalibration>  calibration_;
  std::shared_ptr<PFEnergyCalibrationHF>  thepfEnergyCalibrationHF_;
  std::shared_ptr<PFSCEnergyCalibration> thePFSCEnergyCalibration_;

  bool               useHO_;
  const bool         debug_;

  PFMuonAlgo *pfmu_;


  /// Variables for NEW EGAMMA selection
  bool useEGammaFilters_;
  bool useProtectionsForJetMET_;
  const edm::View<reco::PFCandidate> * pfEgammaCandidates_;
  const edm::ValueMap<reco::GsfElectronRef> * valueMapGedElectrons_;
  const edm::ValueMap<reco::PhotonRef> * valueMapGedPhotons_;  


  // Option to let PF decide the muon momentum
  bool usePFMuonMomAssign_;

  /// Flags to use the protection against fakes 
  /// and not reconstructed displaced vertices
  bool rejectTracks_Bad_;
  bool rejectTracks_Step45_;

  bool usePFNuclearInteractions_;
  bool usePFConversions_;
  bool usePFDecays_;

  /// Maximal relative uncertainty on the tracks going to or incoming from the 
  /// displcaed vertex to be used in the PFAlgo
  double dptRel_DispVtx_;
  int nVtx_;

  /// A tool used for a postprocessing of displaced vertices
  /// based on reconstructed PFCandidates
  PFCandConnector connector_;
    
  /// Variables for muons and fakes
  std::vector<double> muonHCAL_;
  std::vector<double> muonECAL_;
  std::vector<double> muonHO_;
  double nSigmaTRACK_;
  double ptError_;
  std::vector<double> factors45_;

  /// Variables for track cleaning in bad HCal areas
  float goodTrackDeadHcal_ptErrRel_;
  float goodTrackDeadHcal_chi2n_;
  int   goodTrackDeadHcal_layers_;
  float goodTrackDeadHcal_validFr_;
  float goodTrackDeadHcal_dxy_;

  float goodPixelTrackDeadHcal_minEta_;
  float goodPixelTrackDeadHcal_maxPt_;
  float goodPixelTrackDeadHcal_ptErrRel_;
  float goodPixelTrackDeadHcal_chi2n_;
  int   goodPixelTrackDeadHcal_maxLost3Hit_;
  int   goodPixelTrackDeadHcal_maxLost4Hit_;
  float goodPixelTrackDeadHcal_dxy_;
  float goodPixelTrackDeadHcal_dz_;


  // Parameters for post HF cleaning
  bool postHFCleaning_;
  bool postMuonCleaning_;
  double minHFCleaningPt_;
  double minSignificance_;
  double maxSignificance_;
  double minSignificanceReduction_;
  double maxDeltaPhiPt_;
  double minDeltaMet_;
  double useBestMuonTrack_;

  //MIKE -May19th: Add option for the vertices....
  reco::Vertex       primaryVertex_;
  bool               useVertices_; 

  edm::Handle<reco::MuonCollection> muonHandle_;

};


#endif
