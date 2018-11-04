#ifndef DESCRIPTIONS_H
# define DESCRIPTIONS_H

# include <stdint.h>
# undef swap
# include <vector>
# include <string>
# include "dice.h"
# include "npc.h"


typedef struct dungeon dungeon_t;

uint32_t parse_descriptions(dungeon_t *d);
uint32_t print_descriptions(dungeon_t *d);
uint32_t destroy_descriptions(dungeon_t *d);

typedef enum object_type {
  objtype_no_type,
  objtype_WEAPON,
  objtype_OFFHAND,
  objtype_RANGED,
  objtype_LIGHT,
  objtype_ARMOR,
  objtype_HELMET,
  objtype_CLOAK,
  objtype_GLOVES,
  objtype_BOOTS,
  objtype_AMULET,
  objtype_RING,
  objtype_SCROLL,
  objtype_BOOK,
  objtype_FLASK,
  objtype_GOLD,
  objtype_AMMUNITION,
  objtype_FOOD,
  objtype_WAND,
  objtype_CONTAINER
} object_type_t;

extern const char object_symbol[];


//Added by LC -------------------------------------------------------------------------

class object{
  public:
    std::string name, description;
    object_type_t type;
    uint32_t color,hit, dodge, defence, weight, speed, attribute, value;
    dice damage;
    bool artifact;
    uint32_t rarity;
  public:
   object() : name(),    description(), type(objtype_no_type),
                         color(0),  hit(),         dodge(),
                         defence(),   weight(),     speed(),
                         attribute(),   value(),   damage(),
                         artifact(false), rarity(0)
  {
  }
  void set(const std::string &name,
           const std::string &description,
           const object_type_t type,
           const uint32_t color,
           const uint32_t hit,
           const dice &damage,
           const uint32_t dodge,
           const uint32_t defence,
           const uint32_t weight,
           const uint32_t speed,
           const uint32_t attribute,
           const uint32_t value,
           const bool artifact,
           const uint32_t rarity);
  std::ostream &print(std::ostream &o);
  /* Need all these accessors because otherwise there is a *
   * circular dependancy that is difficult to get around.  */
  inline const std::string &get_name() const { return name; }
  inline const std::string &get_description() const { return description; }
  inline const object_type_t get_type() const { return type; }
  inline const uint32_t get_color() const { return color; }
  inline const uint32_t get_hit() const { return hit; }
  inline const dice &get_damage() const { return damage; }
  inline const uint32_t get_dodge() const { return dodge; }
  inline const uint32_t get_defence() const { return defence; }
  inline const uint32_t get_weight() const { return weight; }
  inline const uint32_t get_speed() const { return speed; }
  inline const uint32_t get_attribute() const { return attribute; }
  inline const uint32_t get_value() const { return value; }

};




class monster_description : public npc {
 private:
  std::string name, description;
  char symbol;
  std::vector<uint32_t> color;
  uint32_t abilities;
  dice speed, hitpoints, damage;
  uint32_t rarity;
 public:
  monster_description() : name(),       description(), symbol(0),   color(0),
                          abilities(0), speed(),       hitpoints(), damage(),
                          rarity(0)
  {
  }
  void set(const std::string &name,
           const std::string &description,
           const char symbol,
           const std::vector<uint32_t> &color,
           const dice &speed,
           const uint32_t abilities,
           const dice &hitpoints,
           const dice &damage,
           const uint32_t rarity);

 std::ostream &print(std::ostream &o);
  //Added by LC
  npc get_npc(){
    npc temp;
    temp.set(name, 
	     description,
	     symbol,
	     color,
	     speed.roll(),
	     abilities,
	     hitpoints.roll(),
	     damage,
	     rarity);
    return temp;

  }


 
  char get_symbol() { return symbol; }
};

class object_description {
 private:
  std::string name, description;
  object_type_t type;
  uint32_t color;
  dice hit, damage, dodge, defence, weight, speed, attribute, value;
  bool artifact;
  uint32_t rarity;
 public:
  object_description() : name(),    description(), type(objtype_no_type),
                         color(0),  hit(),         damage(),
                         dodge(),   defence(),     weight(),
                         speed(),   attribute(),   value(),
                         artifact(false), rarity(0)
  {
  }
  void set(const std::string &name,
           const std::string &description,
           const object_type_t type,
           const uint32_t color,
           const dice &hit,
           const dice &damage,
           const dice &dodge,
           const dice &defence,
           const dice &weight,
           const dice &speed,
           const dice &attribute,
           const dice &value,
           const bool artifact,
           const uint32_t rarity);


  //added by LC
  object get_object(){
    object temp;
    temp.set(name,  
	       description,
	       type,
	       color,
	       hit.roll(),
	       damage,
	       dodge.roll(),
	       defence.roll(),
	       weight.roll(),
	       speed.roll(),
	       attribute.roll(),
	       value.roll(),
	       artifact,
	       rarity);
    return temp;

  };



  std::ostream &print(std::ostream &o);
  /* Need all these accessors because otherwise there is a *
   * circular dependancy that is difficult to get around.  */
  inline const std::string &get_name() const { return name; }
  inline const std::string &get_description() const { return description; }
  inline const object_type_t get_type() const { return type; }
  inline const uint32_t get_color() const { return color; }
  inline const dice &get_hit() const { return hit; }
  inline const dice &get_damage() const { return damage; }
  inline const dice &get_dodge() const { return dodge; }
  inline const dice &get_defence() const { return defence; }
  inline const dice &get_weight() const { return weight; }
  inline const dice &get_speed() const { return speed; }
  inline const dice &get_attribute() const { return attribute; }
  inline const dice &get_value() const { return value; }


       



};

std::ostream &operator<<(std::ostream &o, monster_description &m);
std::ostream &operator<<(std::ostream &o, object_description &od);
//added by LC
std::ostream &operator<<(std::ostream &o, object &oi);

#endif
