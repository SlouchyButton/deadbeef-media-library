dependencies = [
    dependency('gtkmm-3.0'),
    dependency('taglib'),
    dependency('boost', modules : [
        'serialization',
        'system',
        'filesystem',
    ])
]

# Apparently deadbeef can't be found with pkg-conf
add_project_arguments('-I\usr\include\deadbeef', language: 'c')

source_files = files(
    'plugin.cpp',
    'filebrowser.cpp',
    'utils.cpp',
    'benchmark.cpp',
    'librarycontroller.cpp',
    'gui/settings/window.cpp',
    'gui/settings/pathscolumns.cpp',
    'gui/modelcolumns.cpp',
    'gui/container.cpp',
    'gui/iconview.cpp',
    'gui/treefilebrowser.cpp',
    'gui/searchbar.cpp',
    'gui/filebrowserfilter.cpp',
    'gui/addressbox.cpp',
    'gui/treepopup.cpp',
    'library/album.cpp',
    'library/mediafile.cpp',
    'library/medialibrary.cpp',
    'library/coverimage.cpp',
    'plugin/controller.cpp',
    'metadata/readerfactory.cpp',
    'metadata/reader.cpp',
    'metadata/abstract/id3v2reader.cpp',
    'metadata/implementation/flacreader.cpp',
    'metadata/implementation/mp3reader.cpp',
    'metadata/implementation/oggreader.cpp',
    'metadata/implementation/wavreader.cpp',
)

includes = [
    include_directories('../include'),
    include_directories('../include/gui'),
    include_directories('../include/gui/settings'),
    include_directories('../include/library'),
    include_directories('../include/metadata'),
    include_directories('../include/metadata/abstract'),
    include_directories('../include/metadata/implementation'),
    include_directories('../include/plugin'),
]

lib = shared_library(
    'ddb_misc_media_library',
    include_directories: includes,
    dependencies: dependencies,
    install: true,
    sources: source_files,
)
