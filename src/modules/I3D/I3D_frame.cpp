#include "I3D_frame.h"
#include <glm/gtx/quaternion.hpp>

void I3D_frame::setOn(bool on) {
    if(on)
        _flags |= FRMFLAGS_ON;
    else 
        _flags &= ~FRMFLAGS_ON;   
}

void I3D_frame::addChild(ea::shared_ptr<I3D_frame> child) {
    child->_parent = this;
    child->_flags |= FRMFLAGS_MAT_DIRTY;

    _children.push_back(ea::move(child));
}

void I3D_frame::removeChild(ea::shared_ptr<I3D_frame> child) {
    auto it = ea::find(_children.begin(), _children.end(), child);
    if(it != _children.end()) {
        _children.erase(ea::remove(_children.begin(), _children.end(), child), _children.end());
    }
}

const glm::mat4& I3D_frame::getLocalMatrix() {
    if(!(_flags & FRMFLAGS_MAT_DIRTY))
        return _matrix;

    getMatrix();
    return _matrix;
}

const glm::mat4& I3D_frame::getMatrix() {
    if(!(_flags & FRMFLAGS_MAT_DIRTY))
        return _worldMatrix;
    
    if((_flags & FRMFLAGS_POS_DIRTY) || (_flags & FRMFLAGS_ROT_DIRTY) || (_flags & FRMFLAGS_SCALE_DIRTY)) {
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), _pos);
        glm::mat4 scale = glm::scale(glm::mat4(1.0f), _scale);
        glm::mat4 rot = glm::toMat4(_rot);

        _matrix = translation * rot * scale;
        _flags &= ~(FRMFLAGS_POS_DIRTY | FRMFLAGS_ROT_DIRTY | FRMFLAGS_SCALE_DIRTY);
    }

    if(_parent != nullptr)
        _worldMatrix = _parent->getMatrix() * _matrix;
    else 
        _worldMatrix = _matrix;
    
    _flags &= ~FRMFLAGS_MAT_DIRTY;
    return _worldMatrix;
}

void I3D_frame::setMatrix(const glm::mat4& mat) {
    _matrix = mat;
    propagateDirty();
}

const glm::vec3& I3D_frame::getPos() {
    return _pos;
}

void I3D_frame::setPos(glm::vec3& pos) {
    _flags |= FRMFLAGS_POS_DIRTY;
    _pos = pos;
    propagateDirty();
}

const glm::quat& I3D_frame::getRot() {
    return _rot;
}

void I3D_frame::setRot(const glm::quat& rot) {
    _flags |= FRMFLAGS_ROT_DIRTY;
    _rot = rot;
    propagateDirty();
}

const glm::vec3& I3D_frame::getScale() {
    return _scale;
}

void I3D_frame::setScale(const glm::vec3& scale) {
    _flags |= FRMFLAGS_SCALE_DIRTY;
    _scale = scale;
    propagateDirty();
}

//----------------------------

void I3D_frame::propagateDirty() {
    _flags |= FRMFLAGS_MAT_DIRTY;

    for(const ea::shared_ptr<I3D_frame>& child : _children) {
        if(child) child->propagateDirty();
    }
}