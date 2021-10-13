

import 'dart:ffi'; // For FFI
// For Platform.isX

import 'uncompress_lib_bindings.dart';
import 'package:ffi/ffi.dart' as ffi;


final uncompressBinding = UncompressBinding(
    DynamicLibrary.open("libUncompress.so"));

class UncompressUtil {

  static List<UncompressedRecord> uncompress (List<int> values ) {
    var pointer = intListToArray(values) ;
    var uncompressedPointer = ffi.calloc.allocate<record_t>(50000) ;
    var count = uncompressBinding.uncompress_data(pointer,values.length , uncompressedPointer);
    print("uncompressed data count is $count");
    ffi.calloc.free(pointer) ;
    var results = List.generate(count, (index) {
       var record = uncompressedPointer.elementAt(index).ref ;
       return UncompressedRecord(record.tempe, record.time);
    });
    ffi.calloc.free(uncompressedPointer) ;
    return results ;
  }

  static Pointer<Uint8> intListToArray(List<int> list) {
    final ptr = ffi.calloc.allocate<Uint8>(list.length);
    for (var i = 0; i < list.length; i++) {
      ptr.elementAt(i).value = list[i];
    }
    return ptr;
  }
}
class UncompressedRecord {
  final int temp ;
  final int time ;

  UncompressedRecord(this.temp, this.time);
}

