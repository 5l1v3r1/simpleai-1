#pragma once

#include "common/Vector3f.h"
#include "common/Thread.h"
#include "common/Types.h"
#include "AI.h"
#include <memory>

namespace ai {

/**
 * @brief Defines some standard names for @c ICharacter attributes. None of these must be used. But if you
 * use them, the remote debugger can make use of known values to render more information in to the view.
 */
namespace attributes {
/**
 * @brief Attribute for the name of an entity
 */
const char* const NAME = "Name";
const char* const GROUP = "Group";
const char* const ID = "Id";
const char* const POSITION = "Position";
const char* const SPEED = "Speed";
const char* const ORIENTATION = "Orientation";
}

/**
 * @brief Class that should be extended by the ai controlled entity class.
 *
 * It uses a @c CharacterId to identify the character in the game. The @c AI class
 * has a reference to its controlled @c ICharacter instance.
 *
 * @note Update the values of the @c ICharacter class only in in the @c ICharacter::update
 * method or from within the @c Zone callbacks. Otherwise you will run into race conditions
 * if you run with multiple threads. You can use the atomic macros from common/Thread.h to
 * add more attributes to your implementation of the @c ICharacter class that you would like
 * to modify in your own update methods and from within the AI.
 *
 * You often need access to your world your character is living in. You need access to this
 * data to resolve the @c CharacterId's in the @c IFilter implementations, to interact with
 * other entities that are not SimpleAI controlled and so on. You can use the provided
 * @c character_cast function in your @c TreeNode, @c IFilter or @¢ ICondition implementations.
 */
class ICharacter : public NonCopyable {
protected:
	CharacterId _id;
	Vector3f _position;
	ATOMIC(float) _orientation;
	// m/s
	ATOMIC(float) _speed;
	CharacterAttributes _attributes;

public:
	ICharacter(CharacterId id) :
			_id(id), _orientation(0.0f), _speed(0.0f) {
	}

	virtual ~ICharacter() {
	}

	bool operator ==(const ICharacter& character) const;
	bool operator !=(const ICharacter& character) const;

	CharacterId getId() const;
	void setPosition(const Vector3f& position);
	void setPosition(Vector3f&& position);

	Vector3f getPosition() const;
	void setOrientation(float orientation);
	/**
	 * @return the radians around the y (up) axis
	 */
	float getOrientation() const;
	void setSpeed(float speed);
	float getSpeed() const;
	void setAttribute(const std::string& key, const std::string& value);
	const CharacterAttributes& getAttributes() const;
	/**
	 * @brief override this method to let your own @c ICharacter implementation
	 * tick with the @c Zone::update
	 *
	 * @param dt the time delta in millis since the last update was executed
	 * @param debuggingActive @c true if the debugging for this entity is activated
	 */
	virtual void update(int64_t dt, bool debuggingActive) {
		(void)dt;
		(void)debuggingActive;
	}
};

inline void ICharacter::setPosition(const Vector3f& position) {
	ai_assert(!position.isInfinite(), "invalid position");
	_position = position;
}

inline void ICharacter::setPosition(Vector3f&& position) {
	ai_assert(!position.isInfinite(), "invalid position");
	_position = std::move(position);
}

inline void ICharacter::setOrientation (float orientation) {
	_orientation = orientation;
}

inline float ICharacter::getOrientation () const {
	return _orientation;
}

inline void ICharacter::setAttribute(const std::string& key, const std::string& value) {
	_attributes[key] = value;
}

inline const CharacterAttributes& ICharacter::getAttributes() const {
	return _attributes;
}

inline bool ICharacter::operator ==(const ICharacter& character) const {
	return character._id == _id;
}

inline bool ICharacter::operator !=(const ICharacter& character) const {
	return character._id != _id;
}

inline CharacterId ICharacter::getId() const {
	return _id;
}

inline Vector3f ICharacter::getPosition() const {
	return _position;
}

inline void ICharacter::setSpeed(float speed) {
	_speed = speed;
}

inline float ICharacter::getSpeed() const {
	return _speed;
}

typedef std::shared_ptr<ICharacter> ICharacterPtr;

template <typename CharacterType>
inline const CharacterType& character_cast(const ICharacter& character) {
	return static_cast<const CharacterType&>(character);
}

template <typename CharacterType>
inline CharacterType& character_cast(ICharacter& character) {
	return static_cast<CharacterType&>(character);
}

template <typename CharacterType>
inline CharacterType& character_cast(const ICharacterPtr& character) {
	return *static_cast<CharacterType*>(character.get());
}

}
