#include "Light.h"
#include "CameraManager.h"
#include "VertexBufferObject.h"
#include "Mesh.h"
#include "Shader.h"
#include "FrameBufferObject.h"
#include "Scene.h"
#include "RenderContext.h"

#include "opengl_utils.h"

Light::Light(Scene *scene, const glm::vec3 &pos)
: m_position(pos),
  m_renderSize(1.0f),
  m_record(false),
  m_first(true),
  m_saved(true),
  m_moveMode(0),
  m_height(20),
  m_angle(0.0f),
  m_radius(20.0f),
  m_time(0),
  m_distance(1.0f),
  m_movement(0.0f),
  m_ncpLight(1.0f),
  m_fcpLight(200.0f),
  m_fovLight(90.0f),
  m_bufferWidth(2048),
  m_bufferHeight(2048),
  m_scene(scene)
{
    loadPaths();

    m_vbo = Mesh::sphere(0.5f, 4, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

    m_shader = new Shader("../Shader/Default33.vert.glsl", "../Shader/Default33.frag.glsl");
    m_shader->bindAttribLocations();

	m_shaderBlur = new Shader("../Shader/Blur.vert.glsl", "../Shader/Blur.frag.glsl");
	m_shaderBlur->bindAttribLocations();

    m_fboLight = new FrameBufferObject(m_bufferWidth, m_bufferHeight);
    glBindTexture(GL_TEXTURE_2D, m_fboLight->texAttachment(GL_DEPTH_ATTACHMENT));
        //glTexParameteri( GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE_ARB );	
	    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_GEQUAL);
       //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);    
    glBindTexture(GL_TEXTURE_2D, 0);

	m_fboBlurH = new FrameBufferObject(m_bufferWidth, m_bufferHeight, 1, 0, GL_FALSE, GL_LINEAR);
    m_fboBlurV = new FrameBufferObject(m_bufferWidth, m_bufferHeight, 1, 0, GL_FALSE, GL_LINEAR);

    m_vboBlur = Mesh::quad(0, 0, m_bufferWidth, m_bufferHeight);
}

Light::~Light()
{
    delete m_shader;
    delete m_vboBlur;
    delete m_shaderBlur;
    delete m_vbo;
    delete m_fboLight;
    delete m_fboBlurV;
    delete m_fboBlurH;   
}

void Light::blurShadowMap()
{
	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.0f));
	glm::mat4 projection = glm::ortho(0.0f, float(m_bufferWidth), 0.0f, float(m_bufferHeight), -1.0f, 1.0f);

    m_fboBlurH->bind();

        glViewport(0, 0, m_bufferWidth, m_bufferHeight);
        glClearColor(1, 0, 0, 1);    
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     

        m_shaderBlur->bind();

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_fboLight->texAttachment(GL_COLOR_ATTACHMENT0));                    
            m_shaderBlur->seti("tex", 0); 
        
            m_shaderBlur->setMatrix("matModel", model, GL_TRUE); 
	        m_shaderBlur->setMatrix("matView", view, GL_TRUE);
	        m_shaderBlur->setMatrix("matProjection", projection, GL_TRUE);
            m_shaderBlur->seti("horizontal", 1);
            m_vboBlur->render();

        m_shaderBlur->release();

    m_fboBlurH->release();

    m_fboBlurV->bind();

        glViewport(0, 0, m_bufferWidth, m_bufferHeight);
        glClearColor(1, 0, 0, 1);    
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     

        m_shaderBlur->bind();

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_fboBlurH->texAttachment(GL_COLOR_ATTACHMENT0));            
            m_shaderBlur->seti("tex", 0); 
        
            m_shaderBlur->setMatrix("matModel", model, GL_TRUE); 
	        m_shaderBlur->setMatrix("matView", view, GL_TRUE);
	        m_shaderBlur->setMatrix("matProjection", projection, GL_TRUE);
            m_shaderBlur->seti("horizontal", 0);
            m_vboBlur->render();

        m_shaderBlur->release();

    m_fboBlurV->release();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);         
}

void Light::setLightView()
{
	auto param = RenderContext::globalObjectParam();

	if (param->updateShadow)
	{
		auto trans = RenderContext::transform();

		glm::mat4 projection = glm::perspective(m_fovLight, (float)m_bufferWidth / (float)m_bufferHeight, m_ncpLight, m_fcpLight);
		glm::mat4 view = glm::lookAt(param->lightPos, glm::vec3(0, 5, 0), glm::vec3(0, 1, 0));

		glm::mat4 bias(0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.5f, 0.0f,
			0.5f, 0.5f, 0.5f, 1.0f);

		trans->lightView = view;
		trans->lightProjection = projection;
		trans->lightViewProjection = projection * view;
		trans->lightViewProjectionBias = bias * projection * view;
	}
}

void Light::renderLightView()
{
	auto param = RenderContext::globalObjectParam();

    if(param->updateShadow)
	{
        m_fboLight->bind();
            
            glViewport(0, 0, m_bufferWidth, m_bufferHeight);
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);    
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     

		    m_scene->renderObjectsDepth();        

        m_fboLight->release();

       blurShadowMap();

       param->shadowMapID = m_fboLight->texAttachment(GL_COLOR_ATTACHMENT0);
	   param->shadowMapBlurredID = m_fboBlurV->texAttachment(GL_COLOR_ATTACHMENT0);

	   param->updateShadow = false;
    }
}

void Light::render()
{
	auto param = RenderContext::globalObjectParam();
	auto trans = RenderContext::transform();

    glm::mat4 model = glm::translate(glm::mat4(1.0f), m_position);
    glm::mat4 view = trans->view;
    glm::mat4 projection = trans->projection;

    m_shader->bind();  
	m_shader->setMatrix("matModel", model, GL_FALSE);
	m_shader->setMatrix("matView", view, GL_FALSE);
	m_shader->setMatrix("matProjection", projection, GL_FALSE);

        m_vbo->render();

    m_shader->release();

}

glm::vec3 Light::position()
{
	return m_position;
}

void Light::setPosition(const glm::vec3 &pos)
{
	m_position = pos;
	RenderContext::globalObjectParam()->updateShadow = true;
}

void Light::autoMove()
{
    if(m_moveMode == 1)
    {
        m_position.x = cosf(m_angle) * m_radius;
        m_position.z = sinf(m_angle) * m_radius;

        m_angle+=m_movement;
    }

    if(m_moveMode == 2)
    {
        m_time += m_movement * 0.1;

        if(m_time > 1.0f)
            m_time = 0.0f;

        //glm::vec3 p = m_spline.interpolatedPoint(m_time);

        //m_position = p;
    }    
}

void Light::move(CameraManager *camManager, float diffX, float diffY)
{
    if(m_moveMode == 0)
    {
	    Transform trans;	

	    glm::vec3 dir;
        glm::vec3 right;
        glm::vec3 up;
        glm::vec3 pos;
    	
	    camManager->currentPerspective(trans);
        Common::getCameraFrame(trans, dir, up, right, pos);

	    glm::vec3 movZ;
	    glm::vec3 movX;
	    glm::vec3 movY(0, m_position.y, 0);
    	
	    glm::vec3 onPlane = glm::cross(right, glm::vec3(0, 1, 0));
	    movZ = glm::vec3(0, 0, m_position.z) + onPlane * diffY;
	    movX = glm::vec3(m_position.x, 0, 0) + right * diffX;
    	
	    glm::vec3 result = movX + movY + movZ ;

	    m_position = result;

        if(m_record)
        {
           /* DWORD time = GetTickCount();
            int diff = time - m_oldTime;*/

			int diff = 0;

            if(diff >= 250)
            {
                //m_oldTime = time;
                m_curPath.positions.push_back(m_position);
                
            }
        }

		RenderContext::globalObjectParam()->updateShadow = true;
    }
}

void Light::loadPaths()
{
   /* QDir dir("Data/Light");

    QStringList filters;
    filters << "*.light";

    QFileInfoList list = dir.entryInfoList(filters);
    
    for(int i=0; i<list.size(); ++i)
    {
        QFile file(list[i].absoluteFilePath());
        
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return;

        QTextStream in(&file);

        Path path;
        path.name = list[i].baseName();

        while (!in.atEnd()) 
        {
            QString line = in.readLine();
            QStringList strPos = line.split(" ");

            glm::vec3 p;
            p.x = strPos[0].toFloat();
            p.y = strPos[1].toFloat();
            p.z = strPos[2].toFloat();

            path.positions.push_back(p);
        }  

        file.close();
        m_curPath = path;
    }*/
}

void Light::savePaths()
{
	/*QString folder = "Data/Light/";

	for(uint i=0; i<m_paths.size(); ++i)
	{
		QString name = m_paths[i].name;

		QFile file(folder + name + ".light");

		if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
			return;

		QTextStream out(&file);
		for(uint j=0; j<m_paths[i].positions.size(); ++j)
		{
            glm::vec3 p = m_paths[i].positions[j];
		    out << p.x << " " << p.y << " " << p.z << endl;
		}

        file.close();
	}*/
}

void Light::recordPath(bool record)
{
    /*m_record = record;

    if(m_record && m_first)
    {       
        QDate date = QDate::currentDate(); 
        QTime time = QTime::currentTime();

        int year = date.year();
        int month = date.month();
        int day = date.day();

        int hour = time.hour();
        int minute = time.minute();
        int second = time.second();

        QString sYear   = QString::number(year);
        QString sMonth  = QString::number(month);
        QString sDay    = QString::number(day);
        QString sHour   = QString::number(hour);
        QString sMinute = QString::number(minute);
        QString sSecond = QString::number(second);
        QString sNull   = QString::number(0);

        QString fMonth  = month  < 10 ? sNull + sMonth  : sMonth;
        QString fDay    = day    < 10 ? sNull + sDay    : sDay;
        QString fHour   = hour   < 10 ? sNull + sHour   : sHour;
        QString fMinute = minute < 10 ? sNull + sMinute : sMinute;
        QString fSecond = second < 10 ? sNull + sSecond : sSecond;

        QString name = sYear + fMonth + fDay + "_" + fHour + fMinute + fSecond;

        m_curPath.name = name;
        m_curPath.positions.clear();

        m_first = false;
        m_saved = false;
    }
    else if(record == false && m_saved == false)
    {
        m_paths.push_back(m_curPath);
        savePaths();

        m_saved = true;
        m_first = true;
    }*/
}

void Light::toggleMode()
{
    //if(m_moveMode == 0)
    //{
    //    m_oldPosition = m_position;
    //}

    //m_moveMode ++;
    //if(m_moveMode > 2)
    //{
    //    m_moveMode = 0;
    //}

    //if(m_moveMode == 0)
    //{
    //    m_position = m_oldPosition;
    //}

    //if(m_moveMode == 2)
    //{
    //    m_spline.clear();
    //    for(uint i=0; i<m_curPath.positions.size(); ++i)
    //    {
    //        m_spline.addPoint(m_curPath.positions[i]);
    //    }

    //    m_time = 0;
    //}
}

void Light::update(float delta)
{
    m_movement = m_distance * delta;
    autoMove();
	RenderContext::globalObjectParam()->updateShadow = true;
}

