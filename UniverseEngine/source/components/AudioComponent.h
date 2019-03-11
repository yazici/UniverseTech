#include <cstdio>
#include <string>

namespace uni
{	
	namespace components
	{		
		struct AudioComponent {
		  std::string m_filename;
		  bool m_isPlaying = true;
		  bool m_is3d = true;
		  bool m_isLooping = false;
		  float m_volume = 50.f;
		  int m_channel = -1;
		
		  AudioComponent(std::string path);
		};
	}
}