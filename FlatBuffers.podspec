Pod::Spec.new do |s|
  s.name             = 'FlatBuffers'
  s.version          = '24.12.23'
  s.summary          = 'FlatBuffers: Memory Efficient Serialization Library'

  s.description      = "FlatBuffers is a cross platform serialization library architected for
  maximum memory efficiency. It allows you to directly access serialized
  data without parsing/unpacking it first, while still having great 
  forwards/backwards compatibility."

  s.homepage         = 'https://github.com/google/flatbuffers'
  s.license          = { :type => 'Apache2.0', :file => 'LICENSE' }
  s.author           = { 'mustii' => 'me@mustiikhalil.se' }
  s.source           = { :git => 'https://github.com/google/flatbuffers.git', :tag => "v" + s.version.to_s, :submodules => true }

  s.ios.deployment_target = '11.0'
  s.osx.deployment_target = '10.14'

  s.swift_version = '5.0'
  s.source_files = 'swift/Sources/Flatbuffers/*.swift'
  s.pod_target_xcconfig = {
    'BUILD_LIBRARY_FOR_DISTRIBUTION' => 'YES'
  }
end
