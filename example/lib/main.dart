import 'dart:io';

import 'package:bodycap_uncompress/uncompress_util.dart';
import 'package:bodycap_uncompress_example/slots_data.dart';
import 'package:ext_storage/ext_storage.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:path/path.dart';
void main() {
  runApp(MyApp());
}

class MyApp extends StatefulWidget {
  @override
  _MyAppState createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  // List<UncompressedRecord> results = [];

  @override
  void initState(){

    Future.delayed(Duration(seconds: 1) ,() async{
      await uncompress([...slot1data] , "slot1data1");
    }) ;

    Future.delayed(Duration(seconds: 1) ,() async{
      await uncompress([...slot2data] , "slot2data2");

      // await uncompress([...listData].reversed.toList() , "reversed");
    }) ;

    Future.delayed(Duration(seconds: 1) ,() async{
      await uncompress([...slot3data] , "slot3data3");

      // await uncompress([...listData].reversed.toList() , "reversed");
    }) ;
    super.initState();
  }

  Future uncompress (List<List<int>> listData ,String key) async{
    final String _appDocDir =
    await ExtStorage.getExternalStoragePublicDirectory(
        ExtStorage.DIRECTORY_DOCUMENTS);
    String filePath =
        '$_appDocDir/samples';
    String outputFile = join(filePath, "${key}.csv");
    var dataFile = File(join(outputFile));
    await dataFile.create(recursive: true);

    int currentIndex = 0 ;
    int numberOfMissingData = 0 ;
    int numberOfSyncData = 0 ;
    int numberOfData = 0 ;
      while (true) {
        List<int> values = await Future.delayed(
            Duration(milliseconds: 500), () => listData[currentIndex]);

        // List<int> values = listData[currentIndex] ;

        try {
          // var messages = UncompressUtil.uncompress(values);
          //estimate the time here  ;
          List<UncompressedRecord> messages = UncompressUtil.uncompress(values) ;
          // var messages = await Future.delayed(Duration(milliseconds: 50) , () => UncompressUtil.uncompress(values));
          await Future.delayed(Duration(milliseconds: 500));
          String messageString = "";
        messages.forEach((element) {
          numberOfData++;
          if (element.temp < 0 || element.temp > 10000) {
            numberOfMissingData++;
          }
          if (element.time < 0 || element.time > 1999999999) {
            numberOfSyncData++;
          }
          messageString+="${element.time};${element.temp}\n";
        });
          dataFile.writeAsStringSync(messageString, mode: FileMode.append);

          if (currentIndex + 1 < listData.length) {
            currentIndex++;
            continue;
          }
          break;
        } catch (e) {
          print(e) ;
        }
      }
    print("$key numberOfData $numberOfData numberOfWrongTemps $numberOfMissingData / numberOfWrongTime $numberOfSyncData \n");

  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('uncompress example app'),
        ),
        body: Center(
          child: Center(
            // child: ListView.builder(
            //
            //     itemBuilder: (context, i) =>
            //         Text("${results[i].time} / ${results[i].temp}"),itemCount: results.length
            // ),
          ),
        ),
      ),
    );
  }
}
