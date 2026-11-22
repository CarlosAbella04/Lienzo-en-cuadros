#include "shaderClass.h"
using namespace std;
string get_file_contents(const char* filename)
{
	ifstream in(filename, ios::binary);
	if (in)
	{
		string contents;
		in.seekg(0, ios::end);
		contents.resize(in.tellg());
		in.seekg(0, ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return(contents);
	}
	throw(errno);
}


Shader::Shader(const char* vertexFile, const char* fragmentFile)
{
	string vertexCode = get_file_contents(vertexFile);
	string fragmentCode = get_file_contents(fragmentFile);

	const char* vertexSource = vertexCode.c_str();
	const char* fragmentSource = fragmentCode.c_str();

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);//entero sin signo nombre (tipo de shader)
	glShaderSource(vertexShader, 1, &vertexSource, NULL);//codigo fuente de antes, usando solo una cadena para todo el shader
	glCompileShader(vertexShader);//no puede entender el codigo fuente entonces toca compilarlo en codigo maquina aquí es un shader de vertices


	//se hace lo mismo con el shader de fragmentos
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	//para poder usar ambos shader se encapsulan en program shader
	ID = glCreateProgram();
	glAttachShader(ID, vertexShader);//para agregar un shader al programashader, primero referencia al programa shader y luego se conecta al propio shader
	glAttachShader(ID, fragmentShader);
	glLinkProgram(ID);//se encampsula el programa shader

	glDeleteShader(vertexShader);// se eliminan los sombreadores que se crearon antes porque ya estan en el programa
	glDeleteShader(fragmentShader);
}

void Shader::Activate()
{
	glUseProgram(ID);
}
void Shader::Delete()
{
	glDeleteProgram(ID);
}