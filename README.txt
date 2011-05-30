Requirements:
    _ lipitk
    _ opencv


How to compile?

    * Using cmake:
        _ mkdir build_release
        _ cd build_release
        _ cmake ..
        _ make

	(for debug)
        _ mkdir build_debug
        _ cd build_debug
        _ cmake -DCMAKE_BUILD_TYPE=Debug ..
        _ make


Tips:
    _ make VERBOSE=1


How to run it?
    _ ./blackboard NUMERALS_NUM
