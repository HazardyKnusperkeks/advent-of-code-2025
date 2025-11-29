import qbs

Product {
    name: "AllWarnings"

    property stringList additionalFinalWarnings: []
    property stringList additionalPureWarnings: []
    property stringList commonWarnings: []
    property stringList cxxWarnings: []
    property stringList cWarnings: []

    Properties {
        condition: qbs.toolchain.contains("gcc") && !qbs.toolchain.contains("clang")

        additionalFinalWarnings: [
            "-Wsuggest-final-types",
            "-Wsuggest-final-methods",
        ]

        additionalPureWarnings: [
            "-Wsuggest-attribute=const",
            "-Wsuggest-attribute=pure",
        ]

        commonWarnings: [
            "-pedantic",
            "-Wcast-qual",
            "-Wconversion",
            "-Wdangling-else",
            "-Wdouble-promotion",
            "-Wduplicated-branches",
            "-Wduplicated-cond",
            "-Wenum-compare",
            "-Wfloat-equal",
            "-Wformat=2",
            "-Wformat-nonliteral",
            "-Wformat-overflow=1",
            "-Wformat-security",
            "-Wformat-signedness",
            "-Wformat-truncation=1",
            "-Wlogical-op",
            "-Wmissing-format-attribute",
            "-Wmissing-noreturn",
            "-Wnull-dereference",
            "-Wplacement-new=2",
            "-Wshadow",
            "-Wsign-conversion",
            "-Wsuggest-attribute=cold",
        ]

        cxxWarnings: commonWarnings.concat([
                                               "-Wextra-semi",
                                               "-Wmismatched-tags",
                                               "-Wnoexcept",
                                               "-Wold-style-cast",
                                               "-Woverloaded-virtual",
                                               "-Wredundant-tags",
                                               "-Wuseless-cast",
                                               "-Wsuggest-override",
                                               "-Wzero-as-null-pointer-constant",
                                           ])

        cWarnings: commonWarnings.concat(["-Wenum-conversion"])
    }

    Export {
        Depends { name: "cpp" }

        property bool pureWarnings: false
        property bool finalWarnings: false

        cpp.systemIncludePaths: {
            if ( typeof cpp !== 'object' || cpp === null ) {
                return [];
            } //if ( typeof cpp !== 'object' || cpp === null )

            if ( typeof cpp.includePaths !== 'object' || cpp.includePaths === null ) {
                return [];
            } //if ( typeof cpp.includePaths !== 'object' || cpp.includePaths === null )

            var inc = cpp.includePaths;

            var ret = [];
            var qtRegex = /qtbase\/include/;
            var mocRegex = /\/qt\.headers$/;

            for ( var i = 0; i < inc.length; ++i ) {
                if ( qtRegex.test(inc[i]) || mocRegex.test(inc[i]) ) {
                    ret.push(inc[i]);
                } //if ( qtRegex.test(inc[i]) || mocRegex.test(inc[i]) )
            } //for ( var i = 0; i < inc.length; ++i )

            return ret;
        }

        Properties {
            condition: finalWarnings && pureWarnings

            cpp.cFlags: exportingProduct.cWarnings.concat(exportingProduct.additionalPureWarnings)
            cpp.cxxFlags: exportingProduct.cxxWarnings.concat(exportingProduct.additionalFinalWarnings).concat(exportingProduct.additionalPureWarnings)
        }
        Properties {
            condition: finalWarnings

            cpp.cFlags: exportingProduct.cWarnings
            cpp.cxxFlags: exportingProduct.cxxWarnings.concat(exportingProduct.additionalFinalWarnings)
        }
        Properties {
            condition: pureWarnings

            cpp.cFlags: exportingProduct.cWarnings.concat(exportingProduct.additionalPureWarnings)
            cpp.cxxFlags: exportingProduct.cxxWarnings.concat(exportingProduct.additionalPureWarnings)
        }

        cpp.cFlags: exportingProduct.cWarnings
        cpp.cxxFlags: exportingProduct.cxxWarnings

        Properties {
            condition: qbs.toolchain.contains("gcc") && cpp.cxxStandardLibrary != "libc++" && qbs.buildVariant == "debug"

            cpp.defines: ["_GLIBCXX_ASSERTIONS"]
        }
    }
}
