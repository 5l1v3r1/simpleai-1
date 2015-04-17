#pragma once

#include "tree/Selector.h"

namespace ai {

/**
 * @brief This node executes all the attached children in random order. This composite only
 * fails if all children failed, too. It doesn't continue a node in the state
 * @c TreeNodeStatus::RUNNING. It will always pick a new random node in each tick.
 *
 * http://aigamedev.com/open/article/selector/
 */
class RandomSelector: public Selector {
public:
	SELECTOR_CLASS(RandomSelector)

	TreeNodeStatus execute(const AIPtr& entity, long deltaMillis) override;
};

}
