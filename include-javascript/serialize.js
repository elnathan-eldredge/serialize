Module.onRuntimeInitialized = async () => {
    console.log("beginapi");
    const api = {
	version: Module.cwrap("versiont", "number", []),
//	test: Module.cwrap("test", "number", []),
    };
    console.log(api.version());
    //    console.log(api.tmalloc());
//    var t = Module.test();
};
