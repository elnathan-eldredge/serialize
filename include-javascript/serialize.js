Module.onRuntimeInitialized = async () => {
    console.log("beginapi");
    const api = {
	version: Module.cwrap("versiont", "number", []),
    };
    console.log(api.version());
};
