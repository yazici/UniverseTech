#include <cstdio>
#include <string>

struct AudioComponent {
  std::string m_filename;
  bool m_isPlaying = true;
  bool m_is3d = true;
  bool m_isLooping = false;
  float m_volume = 50.f;

  AudioComponent(std::string path);
};