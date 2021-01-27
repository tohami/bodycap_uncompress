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
    List<int> values = [165, 179, 189, 102, 239, 238, 246, 208, 0, 87, 3, 125, 125, 125, 125, 55, 7, 7, 7, 3, 125, 125, 125, 125, 55, 7, 7, 7, 3, 125, 125, 125, 125, 55, 7, 7, 7, 3, 125, 125, 125, 125, 55, 7, 7, 7, 3, 125, 125, 125, 125, 55, 7, 7, 7, 3, 125, 125, 125, 125, 55, 7, 7, 7, 3, 125, 125, 125, 125, 55, 7, 7, 7, 3, 125, 125, 125, 125, 55, 7, 7, 7, 3, 125, 125, 125, 125, 55, 7, 7, 7, 3, 125, 125, 125, 125, 55, 7, 7, 7, 3, 125, 125, 125, 125, 55, 7, 7, 7, 3, 125, 125, 125, 125, 55, 7, 7, 7, 3, 125, 125, 125, 125, 55, 7, 7, 7, 3, 125, 125, 125, 125, 55, 7, 7, 7, 3, 125, 125, 125, 125, 55, 7, 7, 7, 3, 125, 125, 125, 125, 55, 7, 7, 7, 3, 125, 125, 125, 125, 55, 7, 7, 7, 3, 125, 125, 125, 125, 55, 7, 7, 7, 3, 125, 125, 125, 125, 55, 7, 7, 7, 3, 125, 125, 125, 125, 55, 7, 15];
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
