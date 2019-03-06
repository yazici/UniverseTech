#include <cstdio>
#include <string>

struct AudioComponent {
  std::string m_filename;
  bool m_isPlaying = false;
  bool m_is3d = false;

  AudioComponent(std::string path, bool is3d = false);
};