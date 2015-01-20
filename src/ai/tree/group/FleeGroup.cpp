#include "FleeGroup.h"
#include "common/Math.h"
#include "common/Random.h"
#include "movement/Steering.h"

namespace ai {

TreeNodeStatus FleeGroup::doAction(AI& entity, long deltaMillis) {
	if (_groupId == -1)
		return FAILED;
	ICharacter& chr = entity.getCharacter();

	const movement::GroupFlee w(entity, chr.getSpeed(), _groupId);
	const MoveVector& mv = w.execute();
	if (mv.getVector().isInfinite())
		return FAILED;

	const float deltaSeconds = static_cast<float>(deltaMillis) / 1000.0f;
	chr.setPosition(chr.getPosition() + (mv.getVector() * deltaSeconds));
	chr.setOrientation(static_cast<float>(mv.getVector().orientation()));
	return FINISHED;
}

NODE_FACTORY_IMPL(FleeGroup)

}
