#ifndef SHADER
#define SHADER

#include "Common.h"
#include "Global.h"
#include <experimental/filesystem>


class Shader
{
    public:       
        Shader();
        Shader(const GLchar *vFileName, const GLchar *fFileName, bool autoUpdate = true);
        Shader(const GLchar *vFileName, const GLchar *gFileName, const GLchar *fFileName, bool autoUpdate = true);
        virtual ~Shader();

		void bind();
		void release() const;  
        void link() const;

        void attachVertexShader(const GLchar *fileName);
        void attachControlShader(const GLchar *fileName);
        void attachEvaluationShader(const GLchar *fileName);
        void attachGeometryShader(const GLchar *fileName);
        void attachFragmentShader(const GLchar *fileName);   

        void bindAttribLocation(const GLchar *label, GLuint attribID);

        void seti(const GLchar* label, GLint arg);
        void setf(const GLchar* label, GLfloat arg);
        void set2i(const GLchar* label, GLint arg1, GLint arg2);
        void set2f(const GLchar* label, GLfloat arg1, GLfloat arg2);
		void set2f(const GLchar* label, const glm::vec2 &v);
        void set3i(const GLchar* label, GLint arg1, GLint arg2, GLint arg3);
        void set3f(const GLchar* label, GLfloat arg1, GLfloat arg2, GLfloat arg3);
        void set3f(const GLchar* label, const glm::vec3 &v);
        void set4i(const GLchar* label, GLint arg1, GLint arg2, GLint arg3, GLint arg4);
		void set4f(const GLchar* label, const glm::vec4 &v);
        void set4f(const GLchar* label, GLfloat arg1, GLfloat arg2, GLfloat arg3, GLfloat arg4);
        void set3iv(const GLchar* label, const GLint* args);
        void set3fv(const GLchar* label, const GLfloat* args);
        void setMatrix(const GLchar* label, const GLfloat* m, GLboolean transpose = GL_FALSE);
        void setMatrix(const GLchar* label, const GLdouble* m, GLboolean transpose = GL_FALSE);
        void setMatrix(const GLchar* label, const glm::mat4x4 &mat, GLboolean transpose = GL_FALSE);
        void setMatrix(const GLchar* label, const glm::mat3x3 &mat, GLboolean transpose = GL_FALSE);

        GLuint id() const;

		void bindAttribLocations();

private:
	GLuint m_id;

	const GLchar *m_vFileName;
	const GLchar *m_cFileName;
	const GLchar *m_eFileName;
	const GLchar *m_gFileName;
	const GLchar *m_fFileName;

	GLuint m_vertProg;
	GLuint m_contProg;
	GLuint m_evalProg;
	GLuint m_geomProg;
	GLuint m_fragProg;

	bool m_vertIsDirty;
	bool m_fragIsDirty;
	bool m_geomIsDirty;
	bool m_contIsDirty;
	bool m_evalIsDirty;

	std::experimental::filesystem::file_time_type  m_vOldDateTime;
	std::experimental::filesystem::file_time_type  m_cOldDateTime;
	std::experimental::filesystem::file_time_type  m_eOldDateTime;
	std::experimental::filesystem::file_time_type  m_gOldDateTime;
	std::experimental::filesystem::file_time_type  m_fOldDateTime;
	bool m_firstUpdate;
	bool m_autoUpdate;

	GLuint m_refreshTime;

	const GLchar *readFile(const GLchar *fileName);
	void checkFile(const char *fileName, std::experimental::filesystem::file_time_type &oldTime, GLuint type);
	GLuint compile(const GLchar *source, GLuint type);
	void cleanUp();
	void autoUpdate();

	void initTimer();

	void reload();
};

#endif

