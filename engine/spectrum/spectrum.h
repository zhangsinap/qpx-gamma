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
 *      Qpx::Spectrum::Spectrum generic spectrum type.
 *                       All public methods are thread-safe.
 *                       When deriving override protected methods.
 *
 *      Qpx::Spectrum::Factory creates spectra of appropriate type
 *                               by type name, from template, or
 *                               from file.
 *
 *      Qpx::Spectrum::Registrar for registering new spectrum types.
 *
 ******************************************************************************/

#ifndef PIXIE_SPECTRUM_H
#define PIXIE_SPECTRUM_H

#include <memory>
#include <initializer_list>
#include <boost/thread.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "daq_types.h"
#include "spectrum_template.h"
#include "detector.h"
#include "tinyxml2.h"
#include "custom_logger.h"

//put this somewhere else?
typedef boost::multiprecision::number<boost::multiprecision::cpp_dec_float<15> > PreciseFloat;

namespace Qpx {
namespace Spectrum {

typedef std::pair<std::vector<uint16_t>, uint64_t> Entry;
typedef std::list<Entry> EntryList;
typedef std::pair<uint32_t, uint32_t> Pair;


struct Metadata {
 public:
  std::string name, description, type;
  uint16_t dimensions, bits;
  uint32_t resolution, appearance;
  bool visible;
  std::vector<int16_t> match_pattern, add_pattern;
  std::vector<Gamma::Setting> attributes;
  PreciseFloat total_count;
  uint64_t     max_chan;
  boost::posix_time::time_duration real_time ,live_time;
  boost::posix_time::ptime  start_time;
  std::vector<Gamma::Detector> detectors;

 Metadata() : bits(0), dimensions(0), resolution(0),
    name("uninitialized_spectrum"), total_count(0.0), max_chan(0),
    appearance(0), visible(false) {}
};


class Spectrum
{
public:
  //constructs invalid spectrum by default. Make private?
  Spectrum() {}

public:
  //named constructors, used by factory
  bool from_template(const Template&);
  bool from_xml(tinyxml2::XMLElement*);

  //get count for specific MCA channel in spectrum
  //list takes as many parameters as there are dimensions
  //implemented in children by _get_count
  uint64_t get_count(std::initializer_list<uint16_t> list = {}) const;
  
  //optimized retrieval of whole spectrum as list of Entries (typedef above)
  //parameters take dimensions_ number of ranges of minimum (inclusive) and maximum (exclusive)
  //implemented in children by _get_spectrum and _get_spectrum_update
  std::unique_ptr<EntryList> get_spectrum(std::initializer_list<Pair> list = {});
  void add_bulk(const Entry&);

  //full save with custom format
  void to_xml(tinyxml2::XMLPrinter&) const;

  //export to some format (factory keeps track of file types)
  bool write_file(std::string dir, std::string format) const;
  bool read_file(std::string name, std::string format);

  //for custom saving (implemented in strategies below)
  std::string channels_to_xml() const;
  uint16_t channels_from_xml(const std::string& str);
  
  //retrieve pre-calculated energies for a given channel
  std::vector<double> energies(uint8_t chan = 0) const;
  
  //set and get detectors
  void set_detectors(const std::vector<Gamma::Detector>& dets);

  //feed acquired data to spectrum
  void addSpill(const Spill&);
  void closeAcquisition(); //must call this after completing

  ///////////////////////////////////////////////
  ///////accessors for various properties////////
  ///////////////////////////////////////////////
  Metadata metadata() const;
  
  std::string name() const;
  std::string type() const;
  uint16_t dimensions() const;
  uint16_t bits() const;
  uint32_t resolution() const;

  //change properties - use carefully...
  void set_visible(bool);
  void set_appearance(uint32_t newapp);
  void set_start_time(boost::posix_time::ptime newtime);
  void set_description(std::string newdesc);
  void set_generic_attr(Gamma::Setting setting);


protected:
  ////////////////////////////////////////
  ////////////////////////////////////////
  //////////THIS IS THE MEAT//////////////
  //implement these to make custom types//
  ////////////////////////////////////////
  
  virtual std::string my_type() const {return "INVALID";}
  virtual bool initialize() {return false;}

  virtual bool _write_file(std::string, std::string) const {return false;}
  virtual bool _read_file(std::string, std::string) {return false;}

  virtual uint64_t _get_count(std::initializer_list<uint16_t>) const = 0;
  virtual std::unique_ptr<std::list<Entry>> _get_spectrum(std::initializer_list<Pair>) = 0;
  virtual void _add_bulk(const Entry&) {}

  virtual std::string _channels_to_xml() const = 0;
  virtual uint16_t _channels_from_xml(const std::string&) = 0;

  virtual void pushHit(const Hit&);           //has default behavior
  virtual void addEvent(const Event&) = 0;
  virtual void addStats(const StatsUpdate&);  //has default behavior
  virtual void addRun(const RunInfo&);        //has default behavior
  virtual bool validateEvent(const Event&) const; //has default behavior
  virtual void _closeAcquisition() {}

  virtual void _set_detectors(const std::vector<Gamma::Detector>& dets); //has default behavior

  void recalc_energies();
  Gamma::Setting get_attr(std::string name) const;

  //////////////////////////////
  ///////member variables///////
  //////////////////////////////
  
  mutable boost::shared_mutex mutex_;
  mutable boost::mutex u_mutex_;

  Metadata metadata_;

  uint8_t shift_by_;

  std::vector<std::vector<double> > energies_;
  
  std::map<int, StatsUpdate> start_stats;
  std::list<Event> backlog;
};


class Factory {
 public:
  static Factory& getInstance()
  {
    static Factory singleton_instance;
    return singleton_instance;
  }
  
  Spectrum* create_type(std::string type)
  {
    Spectrum *instance = nullptr;
    auto it = constructors.find(type);
    if(it != constructors.end())
      instance = it->second();
    return instance;
  }

  Spectrum* create_from_template(const Template& tem)
  {
    Spectrum* instance = create_type(tem.type);
    if (instance != nullptr) {
      bool success = instance->from_template(tem);
      if (success)
        return instance;
      else {
        delete instance;
        return nullptr;
      }
    }
  }

  Spectrum* create_from_xml(tinyxml2::XMLElement* root)
  {
    if ((root == nullptr) || ((root->Attribute("type")) == nullptr))
      return nullptr;
    Spectrum* instance = create_type(root->Attribute("type"));
    if (instance != nullptr) {
      bool success = instance->from_xml(root);
      if (success)
        return instance;
      else {
        delete instance;
        return nullptr;
      }
    }
  }
    
  Spectrum* create_from_file(std::string filename)
  {
    std::string ext(boost::filesystem::extension(filename));
    if (ext.size())
      ext = ext.substr(1, ext.size()-1);
    boost::algorithm::to_lower(ext);
    Spectrum *instance = nullptr;
    auto it = ext_to_type.find(ext);
    if (it != ext_to_type.end())
      instance = create_type(it->second);;
    if (instance != nullptr) {
      bool success = instance->read_file(filename, ext);
      if (success)
        return instance;
      else {
        delete instance;
        return nullptr;
      }
    }    
  }

  Template* create_template(std::string type)
  {
    auto it = templates.find(type);
    if(it != templates.end())
      return new Template(it->second);
    else
      return nullptr;
  }

  void register_type(Template tt, std::function<Spectrum*(void)> typeConstructor)
  {
    PL_INFO << "registering type " << tt.type;
    constructors[tt.type] = typeConstructor;
    templates[tt.type] = tt;
    for (auto &q : tt.input_types)
      ext_to_type[q] = tt.type;
  }

  const std::vector<std::string> types() {
    std::vector<std::string> all_types;
    for (auto &q : constructors)
      all_types.push_back(q.first);
    return all_types;
  }

 private:
  std::map<std::string, std::function<Spectrum*(void)>> constructors;
  std::map<std::string, std::string> ext_to_type;
  std::map<std::string, Template> templates;

  //singleton assurance
  Factory() {}
  Factory(Factory const&);
  void operator=(Factory const&);
};

template<class T>
class Registrar {
public:
  Registrar(std::string)
  {
    Factory::getInstance().register_type(T::get_template(),
                                         [](void) -> Spectrum * { return new T();});
  }
};


}}

#endif // PIXIE_SPECTRUM_H