#!/bin/bash
echo
echo PanGet

echo - macdeployqt

cd ~/Development/GitHub/PanGet

rm -R '../../Distribution/PanGet/PanGet.app'
cp -R './build-PanGet-Desktop_Qt_5_9_2_clang_64bit-Release/PanGet.app' '../../Distribution/PanGet/PanGet.app'
cp './trunk/Resources/Info.plist' '../../Distribution/PanGet/PanGet.app/Contents/Info.plist'

/Developer/Qt/5.9.2/clang_64/bin/macdeployqt '../../Distribution/PanGet/PanGet.app'

echo - code signing

codesign --force --verify --sign 'Developer ID Application: Alfred-Wegener-Institut fur Polar- und Meeresforschung (AWI)' '../../Distribution/PanGet/PanGet.app/Contents/Frameworks/QtCore.framework'
codesign --force --verify --sign 'Developer ID Application: Alfred-Wegener-Institut fur Polar- und Meeresforschung (AWI)' '../../Distribution/PanGet/PanGet.app/Contents/Frameworks/QtGui.framework'
codesign --force --verify --sign 'Developer ID Application: Alfred-Wegener-Institut fur Polar- und Meeresforschung (AWI)' '../../Distribution/PanGet/PanGet.app/Contents/Frameworks/QtNetwork.framework'
codesign --force --verify --sign 'Developer ID Application: Alfred-Wegener-Institut fur Polar- und Meeresforschung (AWI)' '../../Distribution/PanGet/PanGet.app/Contents/Frameworks/QtPrintSupport.framework'
codesign --force --verify --sign 'Developer ID Application: Alfred-Wegener-Institut fur Polar- und Meeresforschung (AWI)' '../../Distribution/PanGet/PanGet.app/Contents/Frameworks/QtSvg.framework'
codesign --force --verify --sign 'Developer ID Application: Alfred-Wegener-Institut fur Polar- und Meeresforschung (AWI)' '../../Distribution/PanGet/PanGet.app/Contents/Frameworks/QtWidgets.framework'

codesign --force --verify --sign 'Developer ID Application: Alfred-Wegener-Institut fur Polar- und Meeresforschung (AWI)' '../../Distribution/PanGet/PanGet.app/Contents/PlugIns/bearer/libqcorewlanbearer.dylib'
codesign --force --verify --sign 'Developer ID Application: Alfred-Wegener-Institut fur Polar- und Meeresforschung (AWI)' '../../Distribution/PanGet/PanGet.app/Contents/PlugIns/bearer/libqgenericbearer.dylib'
codesign --force --verify --sign 'Developer ID Application: Alfred-Wegener-Institut fur Polar- und Meeresforschung (AWI)' '../../Distribution/PanGet/PanGet.app/Contents/PlugIns/imageformats/libqgif.dylib'
codesign --force --verify --sign 'Developer ID Application: Alfred-Wegener-Institut fur Polar- und Meeresforschung (AWI)' '../../Distribution/PanGet/PanGet.app/Contents/PlugIns/imageformats/libqicns.dylib'
codesign --force --verify --sign 'Developer ID Application: Alfred-Wegener-Institut fur Polar- und Meeresforschung (AWI)' '../../Distribution/PanGet/PanGet.app/Contents/PlugIns/imageformats/libqico.dylib'
codesign --force --verify --sign 'Developer ID Application: Alfred-Wegener-Institut fur Polar- und Meeresforschung (AWI)' '../../Distribution/PanGet/PanGet.app/Contents/PlugIns/imageformats/libqjpeg.dylib'
codesign --force --verify --sign 'Developer ID Application: Alfred-Wegener-Institut fur Polar- und Meeresforschung (AWI)' '../../Distribution/PanGet/PanGet.app/Contents/PlugIns/imageformats/libqmacjp2.dylib'
codesign --force --verify --sign 'Developer ID Application: Alfred-Wegener-Institut fur Polar- und Meeresforschung (AWI)' '../../Distribution/PanGet/PanGet.app/Contents/PlugIns/imageformats/libqtga.dylib'
codesign --force --verify --sign 'Developer ID Application: Alfred-Wegener-Institut fur Polar- und Meeresforschung (AWI)' '../../Distribution/PanGet/PanGet.app/Contents/PlugIns/imageformats/libqtiff.dylib'
codesign --force --verify --sign 'Developer ID Application: Alfred-Wegener-Institut fur Polar- und Meeresforschung (AWI)' '../../Distribution/PanGet/PanGet.app/Contents/PlugIns/imageformats/libqwbmp.dylib'
codesign --force --verify --sign 'Developer ID Application: Alfred-Wegener-Institut fur Polar- und Meeresforschung (AWI)' '../../Distribution/PanGet/PanGet.app/Contents/PlugIns/imageformats/libqwebp.dylib'
codesign --force --verify --sign 'Developer ID Application: Alfred-Wegener-Institut fur Polar- und Meeresforschung (AWI)' '../../Distribution/PanGet/PanGet.app/Contents/PlugIns/platforms/libqcocoa.dylib'
codesign --force --verify --sign 'Developer ID Application: Alfred-Wegener-Institut fur Polar- und Meeresforschung (AWI)' '../../Distribution/PanGet/PanGet.app/Contents/PlugIns/printsupport/libcocoaprintersupport.dylib'
codesign --force --verify --sign 'Developer ID Application: Alfred-Wegener-Institut fur Polar- und Meeresforschung (AWI)' '../../Distribution/PanGet/PanGet.app/Contents/PlugIns/iconengines/libqsvgicon.dylib'

codesign --force --verify --sign 'Developer ID Application: Alfred-Wegener-Institut fur Polar- und Meeresforschung (AWI)' '../../Distribution/PanGet/PanGet.app'

echo - mount package

cd ~/Development/Distribution/PanGet
hdiutil attach ~/Development/Distribution/PanGet_OSX.dmg

rm -R '/Volumes/PanGet/PanGet.app'
cp -R PanGet.app '/Volumes/PanGet'

cd '/Volumes/PanGet'
rm -rf .fseventsd
mkdir .fseventsd
touch .fseventsd/no_log
cd ~/Development/Distribution/PanGet

echo - verify package

codesign -d '/Volumes/PanGet/PanGet.app'

echo
hdiutil detach '/Volumes/PanGet'

echo - move application

rm -R /Applications/PanGet.app
cp -R PanGet.app /Applications
cd ~/Development/Distribution

echo - finished
