#import "BodycapUncompressPlugin.h"
#if __has_include(<bodycap_uncompress/bodycap_uncompress-Swift.h>)
#import <bodycap_uncompress/bodycap_uncompress-Swift.h>
#else
// Support project import fallback if the generated compatibility header
// is not copied when this plugin is created as a library.
// https://forums.swift.org/t/swift-static-libraries-dont-copy-generated-objective-c-header/19816
#import "bodycap_uncompress-Swift.h"
#endif

@implementation BodycapUncompressPlugin
+ (void)registerWithRegistrar:(NSObject<FlutterPluginRegistrar>*)registrar {
  [SwiftBodycapUncompressPlugin registerWithRegistrar:registrar];
}
@end
