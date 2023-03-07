

import 'dart:ffi'; // For FFI
// For Platform.isX

import 'uncompress_lib_bindings.dart';
import 'package:ffi/ffi.dart' as ffi;


final uncompressBinding = UncompressBinding(
    DynamicLibrary.open("libUncompress.so"));

class UncompressUtil {

  static List<UncompressedRecord> uncompress (List<int> values ) {
      return ffi.using((arena) {
        var uncompressedPointer = arena.allocate<samples_t>(500000);
        var pointer = intListToArray(values, arena);
        var ret = uncompressBinding.lib_uncompress_data(
            pointer, values.length, uncompressedPointer);
        samples_t samples = uncompressedPointer.elementAt(0).ref;
        var results = List.generate(samples.nbSamples, (index) {
          var record = samples.samples[index];
          return UncompressedRecord(record.tempe, record.time);
        });
        arena.releaseAll();
        return results;
      } , ffi.malloc);
  }

  static Pointer<Uint8> intListToArray(List<int> list , ffi.Arena arena) {
    final ptr = arena.allocate<Uint8>(list.length);
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

