import 'package:bodycap_uncompress/uncompress_util.dart';
import 'package:flutter/material.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatefulWidget {
  @override
  _MyAppState createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  List<UncompressedRecord> results = [];

  @override
  void initState() {
    List<int> values = [
      0xA5,
      0xDB,
      0x6F,
      0x98,
      0x4F,
      0xE9,
      0x2B,
      0xD0,
      0x12,
      0xC6,
      0x5B,
      0x76,
      0xCB,
      0xDF,
      0x1E,
      0x09,
      0x3C,
      0x5F,
      0xD1,
      0x8C,
      0xFF,
      0xFF,
      0xF7,
      0x5F
    ];
    try {
      this.results = UncompressUtil.uncompress(values);
    }catch(e) {

    }
    super.initState();
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
            child: ListView.builder(
                itemBuilder: (context, i) =>
                    Text("${results[i].time} / ${results[i].temp}"),itemCount: results.length
            ),
          ),
        ),
      ),
    );
  }
}
