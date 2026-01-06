#include "railway/app/BlockController.h"

namespace railway::app {

BlockController::BlockController(const Config& cfg,
                                 railway::hal::IClock& clock,
                                 railway::drivers::TrackCircuitInput& ownTrack,
                                 railway::drivers::TrackCircuitInput& downstreamTrack,
                                 railway::drivers::SignalHead& signal)
    : cfg_(cfg),
      clock_(clock),
      ownTrack_(ownTrack),
      downstreamTrack_(downstreamTrack),
      signal_(signal) {}

void BlockController::init() {
    ownTrack_.init();
    downstreamTrack_.init();
    signal_.init();

    lastTickMs_ = clock_.nowMs();
    last_ = railway::logic::evaluate(railway::logic::Inputs{});
    signal_.setAspect(last_.aspect);
}

void BlockController::tick() {
    const auto now = clock_.nowMs();

    const bool fresh = (lastTickMs_ == 0) ? true : ((now - lastTickMs_) <= cfg_.maxLoopGapMs);
    lastTickMs_ = now;

    ownTrack_.update(now);
    downstreamTrack_.update(now);

    railway::logic::Inputs in{};
    in.controllerFresh = fresh;
    in.ownTrackCircuitHealthy = ownTrack_.isHealthy();
    in.ownBlockOccupied = ownTrack_.isOccupied();
    in.downstreamBlockOccupied = downstreamTrack_.isOccupied();

    last_ = railway::logic::evaluate(in);
    signal_.setAspect(last_.aspect);
}

railway::logic::Decision BlockController::lastDecision() const {
    return last_;
}

} // namespace railway::app
