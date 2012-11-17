#define CATCH_CONFIG_MAIN

#include "catch.hpp"

/* Internal libraries */
#include "path.hpp"

using namespace apathy;

TEST_CASE("path", "Path functionality works as advertised") {
    SECTION("cwd", "And equivalent vs ==") {
        Path cwd(Path::cwd());
        Path empty("");
        REQUIRE(cwd != empty);
        REQUIRE(cwd.equivalent(empty));
        REQUIRE(empty.equivalent(cwd));
        REQUIRE(cwd.is_absolute());
        REQUIRE(!empty.is_absolute());
        REQUIRE(empty.absolute() == cwd);
        REQUIRE(Path() == ".");
    }

    SECTION("operator=", "Make sure assignment works as expected") {
        Path cwd(Path::cwd());
        Path empty("");
        REQUIRE(cwd != empty);
        empty = cwd;
        REQUIRE(cwd == empty);
    }

    SECTION("operator+=", "Make sure operator<< works correctly") {
        Path root("/");
        root << "hello" << "how" << "are" << "you";
        REQUIRE(root.string() == "/hello/how/are/you");

        /* It also needs to be able to accept things like floats, ints, etc. */
        root = Path("/");
        root << "hello" << 5 << "how" << 3.14 << "are";
        REQUIRE(root.string() == "/hello/5/how/3.14/are");
    }

    SECTION("trim", "Make sure trim actually strips off separators") {
        Path root("/hello/how/are/you////");
        REQUIRE(root.trim().string() == "/hello/how/are/you");
        root = Path("/hello/how/are/you");
        REQUIRE(root.trim().string() == "/hello/how/are/you");
        root = Path("/hello/how/are/you/");
        REQUIRE(root.trim().string() == "/hello/how/are/you");
    }

    SECTION("directory", "Make sure we can make paths into directories") {
        Path root("/hello/how/are/you");
        REQUIRE(root.directory().string() == "/hello/how/are/you/");
        root = Path("/hello/how/are/you/");
        REQUIRE(root.directory().string() == "/hello/how/are/you/");
        root = Path("/hello/how/are/you//");
        REQUIRE(root.directory().string() == "/hello/how/are/you/");
    }

    SECTION("relative", "Evaluates relative urls correctly") {
        Path a("/hello/how/are/you");
        Path b("foo");
        REQUIRE(a.relative(b).string() == "/hello/how/are/you/foo");
        a = Path("/hello/how/are/you/");
        REQUIRE(a.relative(b).string() == "/hello/how/are/you/foo");
        b = Path("/fine/thank/you");
        REQUIRE(a.relative(b).string() == "/fine/thank/you");
    }

    SECTION("parent", "Make sure we can find the parent directory") {
        Path a("/hello/how/are/you");
        REQUIRE(a.parent().string() == "/hello/how/are/");
        a = Path("/hello/how/are/you");
        REQUIRE(a.parent().parent().string() == "/hello/how/");
        
        /* / is its own parent, at least according to bash:
         *
         *    cd / && cd ..
         */
        a = Path("/");
        REQUIRE(a.parent().string() == "/");

        a = Path("");
        REQUIRE(a.parent() == Path::cwd().parent());
    }

    SECTION("makedirs", "Make sure we recursively make directories") {
        Path path("foo");
        REQUIRE(!path.exists());
        path << "bar" << "baz" << "whiz";
        Path::makedirs(path);
        REQUIRE(path.exists());
        REQUIRE(path.is_directory());

        /* Now, we should remove the directories, make sure it's gone. */
        REQUIRE(Path::rmdirs("foo"));
        REQUIRE(!Path("foo").exists());
    }

    SECTION("listdirs", "Make sure we can list directories") {
        Path path("foo");
        path << "bar" << "baz" << "whiz";
        Path::makedirs(path);
        REQUIRE(path.exists());

        /* Now touch some files in this area */
        Path::touch(Path(path).append("a"));
        Path::touch(Path(path).append("b"));
        Path::touch(Path(path).append("c"));

        /* Now list that directory */
        std::vector<Path> files = Path::listdir(path);
        REQUIRE(files.size() == 3);
        REQUIRE(files[0].absolute().string() ==
            Path(path).absolute().append("a").string());
        REQUIRE(files[1].absolute().string() ==
            Path(path).absolute().append("b").string());
        REQUIRE(files[2].absolute().string() ==
            Path(path).absolute().append("c").string());

        REQUIRE(Path::rmdirs("foo"));
        REQUIRE(!Path("foo").exists());
    }

    SECTION("sanitize", "Make sure we can sanitize a path") {
        Path path("foo///bar/a/b/../c");
        REQUIRE(path.sanitize().string() == "foo/bar/a/c");

        path = "../foo///bar/a/b/../c";
        REQUIRE(path.sanitize().string() == Path::cwd().parent(
            ).append("foo").append("bar").append("a").append("c").string());

        path = "../../a/b////c";
        REQUIRE(path.sanitize().string() == Path::cwd().parent().parent(
            ).append("a").append("b").append("c").string());

        path = "/../../a/b////c";
        REQUIRE(path.sanitize().string() == "/a/b/c");

        path = "/./././a/./b/../../c";
        REQUIRE(path.sanitize().string() == "/c");

        path = "././a/b/c/";
        REQUIRE(path.sanitize().string() == Path::cwd().append("a").append(
            "b").append("c").directory().string());
    }

    SECTION("equivalent", "Make sure equivalent paths work") {
        Path a("foo////a/b/../c/");
        Path b("foo/a/c/");
        REQUIRE(a.equivalent(b));

        a = "../foo/bar/";
        b = Path::cwd().parent().append("foo").append("bar").directory();
        REQUIRE(a.equivalent(b));
    }

    SECTION("split", "Make sure we can get segments out") {
        Path a("foo/bar/baz");
        std::vector<Path::Segment> segments(a.split());
        REQUIRE(segments.size() == 3);
        REQUIRE(segments[0].segment == "foo");
        REQUIRE(segments[1].segment == "bar");
        REQUIRE(segments[2].segment == "baz");
    }
}
