#include <Python.h>
#include <exception>
#include <iostream>
#include <memory>
#include <string>

class PythonError : std::exception {
public:
    PythonError()
    {
        PyObject *ptype {}, *pvalue {}, *ptraceback {};
        PyErr_Fetch(&ptype, &pvalue, &ptraceback);
        mError = pvalue ? PyUnicode_AsUTF8(pvalue) : "unknown error";
    }

    const char* what() const noexcept override
    {
        return mError.c_str();
    }

private:
    std::string mError;
};

struct PyObjectDeleter {
    void operator()(PyObject* p) const
    {
        Py_DECREF(p);
    }
};

using UniquePyObj = std::unique_ptr<PyObject, PyObjectDeleter>;

UniquePyObj wrapPy(PyObject* p)
{
    return UniquePyObj { p };
}

UniquePyObj wrapPyOrExit(PyObject* p)
{
    if (!p) {
        throw PythonError {};
    }
    return UniquePyObj { p };
}

class PyInitGuard {
public:
    PyInitGuard()
    {
        Py_Initialize();

        PyObject* sysPath = PySys_GetObject((char*)"path");
        auto programName = wrapPyOrExit(PyUnicode_FromString("."));
        PyList_Append(sysPath, programName.get());
    }

    ~PyInitGuard()
    {
        Py_Finalize();
    }
};

int callIntFunc(const std::string& proc, int param)
{
    auto pModule = wrapPyOrExit(PyImport_ImportModule("PythonCode"));
    auto pDict = wrapPyOrExit(PyModule_GetDict(pModule.get()));
    auto pFunc = wrapPyOrExit(PyDict_GetItemString(pDict.get(), proc.c_str()));

    if (PyCallable_Check(pFunc.get())) {
        auto pValue = wrapPy(Py_BuildValue("(i)", param));
        auto pResult = wrapPy(PyObject_CallObject(pFunc.get(), pValue.get())); // this is wrong tuple is expected here

        return _PyLong_AsInt(pResult.get());
    }
    PyErr_Print();

    return -10;
}

int main()
{
    try {
        PyInitGuard initGuard;
        std::cout << callIntFunc("MultiplicationTable", 3) << '\n';
    } catch (const std::exception& e) {
        std::cout << e.what() << '\n';
    }
}
