/*******************************************************************************
 *
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 *
 * This software can be redistributed and/or modified freely provided that
 * any derivative works bear some notice that they are derived from it, and
 * any modified versions bear some notice that they have been modified.
 *
 * Author(s):
 *      Martin Shetty (NIST)
 *
 * Description:
 *      Pixie::Settings online and offline setting describing the state of
 *      a Pixie-4 device.
 *      Not thread safe, mostly for speed concerns (getting stats during run)
 *
 ******************************************************************************/

#ifndef PIXIE_SETTINGS
#define PIXIE_SETTINGS

#include "detector.h"
#include "generic_setting.h"  //make full use of this!!!
#include "tinyxml2.h"

namespace Pixie {

enum class LiveStatus : int {dead = 0, online = 1, offline = 2, history = 3};
enum class Module     : int {all = -3, none = -2, invalid = -1};
enum class Channel    : int {all = -3, none = -2, invalid = -1};

class Wrapper; //forward declared for friendship

class Settings {
  //Only wrapper can boot, inducing live_==online state, making it possible
  //to affect the device itself. Otherwise Settings is in dead or history state
  friend class Pixie::Wrapper;
  
public:

  Settings();
  Settings(const Settings& other);      //sets state to history if copied
  Settings(tinyxml2::XMLElement* root); //create from xml node

  LiveStatus live() {return live_;}

  //save
  void to_xml(tinyxml2::XMLPrinter&) const;
  
  //detectors
  std::vector<Gamma::Detector> get_detectors() const {return detectors_;}
  void set_detector(int, Gamma::Detector);

  void save_optimization(Channel chan = Channel::all);  //specify module as well?
  void load_optimization(Channel chan = Channel::all);
  void set_setting(Gamma::Setting address, int index);
  Gamma::Setting get_setting(Gamma::Setting address, int index) const;

  /////SETTINGS/////
  Gamma::Setting pull_settings();
  void push_settings(const Gamma::Setting&);
  bool write_settings_bulk();
  bool read_settings_bulk();
  bool write_detector(const Gamma::Setting &set);
  
  void get_all_settings();
  void reset_counters_next_run();
  

  
protected:
  bool boot();       //only wrapper can use this
  void from_xml(tinyxml2::XMLElement*);

  Gamma::Setting settings_tree_;
  
  int         num_chans_;
  LiveStatus  live_;

  std::vector<std::string> boot_files_;
  std::vector<double> system_parameter_values_;
  std::vector<double> module_parameter_values_;
  std::vector<double> channel_parameter_values_;

  std::vector<Gamma::Detector> detectors_;

  //////////for internal use only///////////
  void save_det_settings(Gamma::Setting&, const Gamma::Setting&, int) const;
  void load_det_settings(Gamma::Setting, Gamma::Setting&, int);

  //carry out task
  bool write_sys(const char*);
  bool write_mod(const char*, uint8_t);
  bool write_chan(const char*, uint8_t, uint8_t);
  bool read_sys(const char*);
  bool read_mod(const char*, uint8_t);
  bool read_chan(const char*, uint8_t, uint8_t);
  
  //find index
  uint16_t i_sys(const char*) const;
  uint16_t i_mod(const char*) const;
  uint16_t i_chan(const char*) const;
  
  //print error
  void set_err(int32_t);
  void boot_err(int32_t);



  //system
  void set_sys(const std::string&, double);
  double get_sys(const std::string&);
  void get_sys_all();

  //module
  void set_mod(const std::string&, double, Module mod);
  double get_mod(const std::string&, Module mod) const;
  double get_mod(const std::string&, Module mod,
                 LiveStatus force = LiveStatus::offline);
  void get_mod_all(Module mod);
  void get_mod_stats(Module mod);

  //channel
  void set_chan(const std::string&, double val,
                Channel channel,
                Module  module);
  void set_chan(uint8_t setting, double val,
                Channel channel,
                Module  module);
  double get_chan(const std::string&,
                  Channel channel,
                  Module  module) const;
  double get_chan(const std::string&,
                  Channel channel,
                  Module  module,
                  LiveStatus force = LiveStatus::offline);
  double get_chan(uint8_t setting,
                  Channel channel,
                  Module  module) const;
  double get_chan(uint8_t setting,
                  Channel channel,
                  Module  module,
                  LiveStatus force = LiveStatus::offline);
  void get_chan_all(Channel channel,
                    Module  module);
  void get_chan_stats(Module  module);
};

}

#endif
