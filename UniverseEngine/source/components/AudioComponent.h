#include <cstdio>
#include <string>

struct AudioComponent {
  std::string m_filename;
  bool m_isLoaded = false;
  bool m_isPlaying = false;
  bool m_is3d = false;

  AudioComponent(std::string filename, bool is3d = false) {
    m_filename = filename;
    m_is3d = is3d;
  }
};