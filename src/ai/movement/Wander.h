#pragma once

#include "Steering.h"
#include "common/Random.h"

namespace ai {
namespace movement {

/**
 * @brief Moves forward in the direction the character is currently facing into.
 *
 * Changes orientation (resp. rotation) in a range of [-rotation,rotation] where more
 * weight is given to keep the current orientation.
 */
class Wander: public ISteering {
protected:
	const float _rotation;
public:
	STEERING_FACTORY

	Wander(const std::string& parameter) :
			ISteering(), _rotation(parameter.empty() ? ai::toRadians(10.0f) : Str::strToFloat(parameter)) {
	}

	MoveVector execute (const AIPtr& ai, float speed) const override {
		const Vector3f& v = Vector3f::fromRadians(ai->getCharacter()->getOrientation()) * speed;
		const MoveVector d(v, ai::randomBinomial() * _rotation);
		return d;
	}

	std::ostream& print(std::ostream& stream, int level) const override {
		for (int i = 0; i < level; ++i) {
			stream << '\t';
		}
		stream << "Wander(" << _rotation << ")";
		return stream;
	}
};

}
}
