class AddonClass {
 public:
  //general members
  bool            _is_fragmented  = false;
  Fragment      * _fragment       = nullptr;
  FragmentCache * _fragment_cache = nullptr;
  const void    * _owner          = nullptr;

  //table members
  std::map<uint8_t, bool> _is_field = {};
  std::map<uint8_t, bool> _is_vector = {};  

  //general methods
  static AddonClass* Get(const void* owner);
  static AddonClass* Get(const Table* owner) {
    return Get(reinterpret_cast<const void*>(owner));
  }
  static void Delete(const void* owner) {
    delete Get(owner);
    map()->erase(owner);
  }
  static void Delete(const Table * owner) {
    Delete(reinterpret_cast<const void*>(owner));
  }

  void SetFragment(Fragment * fragment);
  bool CheckByte(const uint32_t byte);

  //vector methods
  const base_t * Get(uoffset_t i);

  //table methods
  const uint8_t * GetVTable();
  const base_t  * GetField(const voffset_t field, const voffset_t field_offset, const uint32_t size);
  const base_t  * GetPointer(const voffset_t field, voffset_t field_offset);

 private:
  //general members
  static std::map<const void*, AddonClass*> * map() {
    static std::map<const void*, AddonClass*> _map = {};
    return &_map;
  }

  //table members
  base_t * vtable_fragment = nullptr;

  //vector methods
  uint32_t VectorElement(const int16_t index) const;

  //table methods
  uint32_t TableElement(const voffset_t field, 
    const voffset_t field_offset);
};

inline AddonClass* AddonClass::Get(const void* owner) {
  if (map()->find(owner) == map()->end()) {
    AddonClass * addon_class     = new AddonClass();
    addon_class->_fragment_cache = new FragmentCache();
    addon_class->_owner          = owner;
    map()->operator[](owner)     = addon_class;
    return addon_class;
  }
  return map()->operator[](owner);
}