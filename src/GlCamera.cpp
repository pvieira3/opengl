#include "GlCamera.h"
#include "glutils.h"
#include <iostream>

GlCamera::GlCamera()
{
    setViewport(0, 0, 100, 100);
}

void GlCamera::rotate(int x, int y)
{
    Eigen::Vector3f cur, origin;

    std::cerr << "\n\n"
              << "xy: " << x << ", " << y
              << "mouse: " << _mouseXY.x() << ", " << _mouseXY.y() << "\n\n";

    cur = hemisphereCoords(x,y);
    origin = hemisphereCoords(_mouseXY.x(), _mouseXY.y());

    std::cerr << "x, y = " << x << ", " << y << "\n"
              << "cur = " << cur.transpose() << "\n"
              << "ori = " << origin.transpose() << "\n";

    Eigen::Quaternionf quat;
//    Eigen::Vector3f axis;
//    float angle;
//    axis = origin.cross(cur);

//    angle = axis.norm() / (origin.norm() * cur.norm());
    quat.setFromTwoVectors(origin, cur);
//    std::cerr << "angle-axis: " << angle << ", " << axis.transpose() << std::endl;
//    std::cerr << "quat      : " << quat.vec().transpose() << std::endl;
//    Eigen::Matrix3f mat = quat.matrix();

    glMatrixMode(GL_MODELVIEW);
    rotateAboutWorld(quat);
   // updateView();
}

void GlCamera::rotate(const Eigen::Quaternionf& quat)
{
    _modelview.rotate(quat);
    loadMatrix(MATRIX_MODELVIEW);
    glutPostRedisplay();
}

void GlCamera::rotateAboutWorld(const Eigen::Quaternionf &quat)
{
//    Eigen::Isometry3f world = Eigen::Isometry3f::Identity();
//    rotateAboutFrame(world, quat);
    _modelview.rotate(quat);
    loadMatrix(MATRIX_MODELVIEW);
    glutPostRedisplay();
}

void GlCamera::rotateAboutFrame(Eigen::Isometry3f& frame, const Eigen::Quaternionf& quat)
{
    Eigen::Isometry3f newPose = frame * quat * _modelview;
    std::cerr << " quat = " << quat.vec().transpose() << "\n"
              << "_pose = \n" << _modelview.matrix() << "\n"
              << "newPz = \n" << newPose.matrix() << std::endl;
    setPose(newPose);

}

void GlCamera::rotateAboutPoint(const Eigen::Vector3f& frame, const Eigen::Quaternionf& quat)
{
//    Eigen::Vector3f newCameraPos = _position - frame;
    //_position = frame + newCameraPos*(quat);
}

void GlCamera::rotate(float x, float y, float z)
{
    glRotatef(x, 1, 0, 0);
    glRotatef(y, 0, 1, 0);
    glRotatef(z, 0, 0, 1);
    updateView();
}

void GlCamera::pan(float x, float y, float z)
{
    pan(Eigen::Vector3f(x,y,z));
}

void GlCamera::pan(const Eigen::Vector3f& p)
{
    _modelview.pretranslate(p);
    loadMatrix(MATRIX_MODELVIEW);
    glutPostRedisplay();
}

void GlCamera::zoom(float factor)
{
    _modelview.pretranslate(Eigen::Vector3f(0, 0, factor));
    loadMatrix(MATRIX_MODELVIEW);
    glutPostRedisplay();

   // setPose(_pose);
//    _position.z() += factor;
//    updateView();
//    glTranslatef(0, 0, factor);
//    glutPostRedisplay();
}

void GlCamera::setCameraType(CameraType cameraType)
{
    _cameraType = cameraType;
}

void GlCamera::setPose(const Eigen::Isometry3f& pose)
{
    _position = pose.translation();
    Eigen::Matrix3f rot = pose.rotation();
    _target << rot(2,0), rot(2,1), rot(2,2);
    _up << rot(1,0), rot(1,1), rot(1,2);
    _modelview = pose;
}

void GlCamera::setPose(const Eigen::Vector3f& position, const Eigen::Vector3f& target, const Eigen::Vector3f& up)
{
    // Set camera position, target and up vectors
    _position = position;
    _target = target;
    _up = up;

    // Set camera pose
        // Set translate part to position of camera in world
    _modelview.setIdentity();
    _modelview.translate(Eigen::Vector3f(position.x(), position.y(), -position.z()));
        // Create rotation part from position, target and up vectors
    Eigen::Vector3f xVec, yVec, zVec;
    zVec = position - target;
    xVec = up.cross(zVec);
    yVec = zVec.cross(xVec);
        // Create rotation matrix and rotate camera pose
    Eigen::Matrix3f rot;
    rot.col(0) = xVec.normalized();
    rot.col(1) = yVec.normalized();
    rot.col(2) = zVec.normalized();
    _modelview.rotate(rot);
    std::cerr << "_modelview:\n" << _modelview.matrix() << std::endl;
}

void GlCamera::setPose()
{
    GLfloat m[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, m);
    _modelview = glutils::glToMat4(m);
    std::cerr << "_modelview:\n" << _modelview.matrix() << std::endl;
    std::cerr << "_modelviewInv:\n" << _modelview.inverse().matrix() << std::endl;
}


//--------------------------
// Private Member Functions
//--------------------------

void GlCamera::setViewport(int x, int y, int w, int h)
{
    _viewport.x = x;
    _viewport.y = y;
    _viewport.w = w;
    _viewport.h = h;
    _viewport.center.x() = (w - x)/2;
    _viewport.center.y() = (h - y)/2;
    _viewport.diagonal = sqrt((w-x)*(w-x) + (h-y)*(h-y));
}

void GlCamera::setMousePosition(const Eigen::Vector2f& position)
{
    _mouseXY = position;
}

Eigen::Vector3f GlCamera::hemisphereCoords(int x, int y) const
{
    std::cerr << "hemicoords input: " << x << ", " << y << std::endl;

    Eigen::Vector3f ray; // Ray in hemisphere going from world origin

    // Compute ray components by first getting radius of hemishpere which
    // is half the diagonal of the viewport
    float r = _viewport.diagonal/2;
    ray.x() = x - _viewport.center.x();
    ray.y() = _viewport.center.y() - y;
    ray.z() = sqrt(r*r - ray.x()*ray.x() - ray.y()*ray.y());

    return ray;
}

void GlCamera::loadMatrix(MatrixType matrix_type)
{
    GLfloat glMat[16];
    glutils::iso3ToGl(getMatrix(matrix_type), glMat);
    glutils::loadMatrix(glMat);
    std::cerr << "modelview\n" << _modelview.matrix() << std::endl;
    std::cerr << "modelviewInv\n" << _modelview.inverse().matrix() << "\n\n";
}

const Eigen::Isometry3f& GlCamera::getMatrix(MatrixType matrix_type)
{
    switch(matrix_type) {
    case MATRIX_PROJECTION:
        //return _projection;
    case MATRIX_MODELVIEW:
    default:
        return _modelview;
    }
}

void GlCamera::updateView()
{
    std::cout << "_position: " << _position.transpose() << "\n"
              <<  " _target: " << _target.transpose() << "\n"
              << "      _up: " << _up.transpose()
              << std::endl;
    gluLookAt(_position.x(), _position.y(), _position.z(),
              _target.x(), _target.y(), _target.z(),
              _up.x(), _up.y(), _up.z());
    glutPostRedisplay();
}
