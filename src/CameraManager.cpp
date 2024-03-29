#include "CameraManager.h"
#include "RenderContext.h"
#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

CameraManager::CameraManager()
: m_active(0),
  m_width(1.0f),
  m_height(1.0f),
  m_aspect(1.0f),
  m_mouseSensivity(0.2f),
  m_camSpeed(10.0f),
  m_useCam(false),
  m_rotate(30.0f, 10.0f, 0.0f),
  m_translate(0.0f, 0.0f, 0.0f),
  m_zoom(20.0),
  m_fov(45.0f),
  m_ncp(1.0f),
  m_fcp(10000.0f)
{
	Camera *cam1 = new Camera(glm::vec3(0, 4, 8), 0, 0, 0, m_fov, m_ncp, m_fcp);
	cam1->setColor(1.0f, 0.0f, 0.0f);
	cam1->setSpeed(m_camSpeed);
	cam1->update();
    cam1->loadFrameDirectory("../Data/Camera");

	m_cameras.push_back(cam1);
}

CameraManager::~CameraManager()
{
	for(int i=0; i<m_cameras.size(); ++i)
	{
		Camera *cam = m_cameras[i];
		delete cam;
	}
}

void CameraManager::currentPerspective(Transform &trans)
{
	if(m_useCam)
	{
        m_cameras[m_active]->determineMovement();        
		m_cameras[m_active]->update();		     
        m_cameras[m_active]->interpolate();
		m_cameras[m_active]->setPerspective(trans);
	}
	else
	{
        m_cameras[m_active]->determineMovement();        
		m_cameras[m_active]->update();	
        m_cameras[m_active]->interpolate();

		glm::mat4 projection = glm::perspective(m_fov, m_aspect, m_ncp, m_fcp);
		glm::mat4 view = glm::mat4(1.0f);

		view = glm::translate(view, glm::vec3(0.0f, 0.0f, -m_zoom));
		view = glm::rotate(view, glm::radians(m_rotate.x), glm::vec3(1.0f, 0.0f, 0.0f));
		view = glm::rotate(view, glm::radians(m_rotate.y), glm::vec3(0.0f, 1.0f, 0.0f));

		trans.projection = projection;
		trans.view = view;
		trans.viewProjection = projection * view;
        trans.normal = glm::transpose( glm::inverse(trans.viewProjection));
	}
}

void CameraManager::resize(float width, float height)
{
	m_aspect = width / height;

	for(int i=0; i<m_cameras.size(); ++i)
	{
		m_cameras[i]->updateCamInternals(m_aspect);
		m_cameras[i]->update();
	}
}

void CameraManager::update()
{
    m_cameras[m_active-1]->determineMovement();
	m_cameras[m_active-1]->update();
}

void CameraManager::renderCameras()
{
	if(!m_useCam)
	{
		for(int i=0; i<m_cameras.size(); ++i)
		{		
			m_cameras[i]->update();
			m_cameras[i]->render();
		}
	}
}

void CameraManager::toggleCam()
{
    if(!m_useCam)
    {
        m_useCam = true;   
    }
    else
    {
        m_active ++;

        if(m_active > (int)m_cameras.size()-1)
        {
            m_active = 0;
            m_useCam = false;
        }
    }
}

void CameraManager::onMouseMove(float dx, float dy, int button)
{
    if(button == 0)
    {
	    if(m_useCam)
	    {
		    m_cameras[m_active]->changeHeading(m_mouseSensivity * dx);
		    m_cameras[m_active]->changePitch(m_mouseSensivity * dy);
	    }
	    else
	    {
            m_rotate.y += (0.1 * dx);
		    m_rotate.x += (0.1 * dy);	
	    }
    }
    else
    {
		    m_cameras[m_active]->changeHeading(m_mouseSensivity * dx);
		    m_cameras[m_active]->changePitch(m_mouseSensivity * dy);
    }
}

void CameraManager::onMouseWheel(int dir)
{
    float delta = m_zoom * -0.1;

	if (dir > 0) 
		m_zoom += delta;			
    else 
	    m_zoom -= delta;	
}

void CameraManager::onKeyPress(int keyId)
{
	switch(keyId)
	{
        case GLFW_KEY_W:
			m_cameras[m_active]->moveForward(true);
            break;
        case GLFW_KEY_S:
			m_cameras[m_active]->moveBackward(true);
            break;
        case GLFW_KEY_A:
			m_cameras[m_active]->strafeLeft(true);
            break;
        case GLFW_KEY_D:
			m_cameras[m_active]->strafeRight(true);
            break;
	}
}

void CameraManager::onKeyRelease(int keyId)
{
	switch(keyId)
	{
		case GLFW_KEY_W:
			m_cameras[m_active]->moveForward(false);
            break;
        case GLFW_KEY_S:
			m_cameras[m_active]->moveBackward(false);
            break;
        case GLFW_KEY_A:
			m_cameras[m_active]->strafeLeft(false);
            break;
        case GLFW_KEY_D:
			m_cameras[m_active]->strafeRight(false);
            break;
	}
}

void CameraManager::increaseSpeed()
{
    m_camSpeed *= 2;

    for(int i=0; i<m_cameras.size(); ++i)
    {
        m_cameras[i]->setDistPerSec(m_camSpeed);
    }
}

void CameraManager::decreaseSpeed()
{
    m_camSpeed /= 2;

    for(int i=0; i<m_cameras.size(); ++i)
    {
        m_cameras[i]->setDistPerSec(m_camSpeed);
    }
}

glm::vec3 CameraManager::lodCamPosition()
{
    return m_cameras[0]->position();
}

Camera *CameraManager::currentCam()
{
    return m_cameras[m_active];
}

void CameraManager::toggleInterpolation()
{
    m_cameras[0]->toggleInterpolation();
}

void CameraManager::addFrame()
{
    m_cameras[0]->autoAddFrame();
}

void CameraManager::clearFrameset()
{
    m_cameras[0]->clearFrames();
}

void CameraManager::saveFrameset()
{
    m_cameras[0]->saveFrames();
}

void CameraManager::toggleFrameset()
{
    m_cameras[0]->changeFrameSet();
}

std::string CameraManager::currentFramesetName()
{
    return m_cameras[0]->frameSetName();
}

Camera *CameraManager::lodCamera()
{
    return m_cameras[0];
}

std::vector<Camera*> CameraManager::cameras()
{
    return m_cameras;
}

glm::vec3 CameraManager::currentCamPos()
{
	if(m_useCam)
	{
		return lodCamPosition();
	} 
	else
	{
		glm::mat4 view = glm::mat4(1.0f);

		view = glm::translate(view, glm::vec3(0.0f, 0.0f, m_zoom));
		view = glm::rotate(view, m_rotate.x, glm::vec3(1.0f, 0.0f, 0.0f));
		view = glm::rotate(view, m_rotate.y, glm::vec3(0.0f, 1.0f, 0.0f)); 

		glm::vec4 camPos = glm::inverse(view) * glm::vec4(0.0, 0.0, 0.0, 1.0);

		return glm::vec3(camPos.x, camPos.y, camPos.z);
	}
}

float CameraManager::currentCamFov()
{
	if(m_useCam)
	{
        return m_cameras[m_active]->fov();
    }
    else
    {
        return m_fov;
    }
}

float CameraManager::currentCamNcp()
{
    if(m_useCam)
	{
        return m_cameras[m_active]->ncp();
    }
    else
    {
        return m_ncp;
    }
}

float CameraManager::currentCamFcp()
{
    if(m_useCam)
	{
        return m_cameras[m_active]->fcp();
    }
    else
    {
        return m_fcp;
    }
    
}

void CameraManager::currentCamParams()
{
	auto param = RenderContext::globalObjectParam();

    if(m_useCam)
    {
        param->fov = m_cameras[m_active]->fov();
        param->ncp = m_cameras[m_active]->ncp();
        param->fcp = m_cameras[m_active]->fcp();
        param->camPos = lodCamPosition();
    }
    else
    {
        param->fov = m_fov;
        param->ncp = m_ncp;
        param->fcp = m_fcp;

		glm::mat4 view = glm::mat4(1.0f);

		view = glm::translate(view, glm::vec3(0, 0, m_zoom));
		view = glm::rotate(view, m_rotate.x, glm::vec3(1, 0, 0));
		view = glm::rotate(view, m_rotate.y, glm::vec3(0, 1, 0)); 

		glm::vec4 camPos = glm::inverse(view) * glm::vec4(0.0, 0.0, 0.0, 1.0);

        param->camPos = glm::vec3(camPos.x, camPos.y, camPos.z);
    }
}