#pragma once

#include "tree/Selector.h"

namespace ai {

/**
 * @brief This node tries to execute all the attached children until one succeeds. This composite only
 * fails if all children failed, too.
 *
 * http://aigamedev.com/open/article/selector/
 */
class PrioritySelector: public Selector {
public:
	SELECTOR_CLASS(PrioritySelector)

	TreeNodeStatus execute(const AIPtr& entity, int64_t deltaMillis) override;
};

}
