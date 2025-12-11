import qbs

Project {
    name: "Advent of Code 2025"

    references: ["allWarnings.qbs"]

    Product {
        name: "eigen"

        Export {
            Depends { name: "cpp" }

            version: "5.0.1"

            cpp.defines: ["EIGEN_FAST_MATH=0"]
            cpp.systemIncludePaths: [exportingProduct.sourceDirectory + "/3rdParty/eigen/"]
        }
    }

    CppApplication {
        consoleApplication: true
        files: [
            "3rdParty/ctre/include/**/*.hpp",
            "astar.hpp",
            "challenge1.cpp",
            "challenge1.hpp",
            "challenge10.cpp",
            "challenge10.hpp",
            "challenge11.cpp",
            "challenge11.hpp",
            "challenge12.cpp",
            "challenge12.hpp",
            "challenge2.cpp",
            "challenge2.hpp",
            "challenge3.cpp",
            "challenge3.hpp",
            "challenge4.cpp",
            "challenge4.hpp",
            "challenge5.cpp",
            "challenge5.hpp",
            "challenge6.cpp",
            "challenge6.hpp",
            "challenge7.cpp",
            "challenge7.hpp",
            "challenge8.cpp",
            "challenge8.hpp",
            "challenge9.cpp",
            "challenge9.hpp",
            "coordinate3d.hpp",
            "helper.cpp",
            "helper.hpp",
            "main.cpp",
            "print.cpp",
            "print.hpp",
        ]

        Depends { name: "AllWarnings" }
        Depends { name: "cpp" }
        Depends { name: "eigen" }

        cpp.cxxLanguageVersion: "c++26"
        cpp.cxxFlags: ["-fconcepts-diagnostics-depth=10"]
    }

    Product {
        files: ["data/*.txt"]
        name: "Data"
    }
}
