
// Small script run *after* the node binary module is either fetched or compiled.

const chalk = require('chalk');

const dllVersion = new Promise((res, rej)=>{
    try {
        const nrfjprog = require('.');
        nrfjprog.getDllVersion((err, version)=>{
            if (err) { rej(err) } else {res(version)}
        })
    } catch(ex) {
        rej(ex);
    }
});


dllVersion
.then((version)=>{
    console.log('nrfjprog libraries at version ', version, ', seems that J-Link libraries are working as expected');
})
.catch((err)=>{
    if (err.errno == 7 &&
        err.errcode === "CouldNotOpenDLL" &&
        err.lowlevelError === "JLINKARM_DLL_NOT_FOUND") {

        console.log();
        console.log(chalk.red.bold("Found error: JLINKARM_DLL_NOT_FOUND"));
        console.log("WARNING: The j-link libraries were not found on your system,");
        console.log("nrfjprog-js cannot function without them.");
        console.log(chalk.bold("Please visit https://www.segger.com/downloads/jlink/ and"));
        console.log(chalk.bold("install the \"J-Link Software and Documentation Pack\"."));
        console.log();
    }
     return Promise.reject(err);

});

