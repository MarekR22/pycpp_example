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

UniquePyObj pyNewRef(PyObject* p)
{
    return UniquePyObj { p };
}

UniquePyObj pyBorrowRef(PyObject* p)
{
    if (p) {
        Py_INCREF(p);
    }
    return pyNewRef(p);
}

void pyRefNotNullThrow(PyObject* p)
{
    if (!p) {
        throw PythonError {};
    }
}

UniquePyObj pyNewRefOrThrow(PyObject* p)
{
    pyRefNotNullThrow(p);
    return pyNewRef(p);
}

UniquePyObj pyBorrowRefOrThrow(PyObject* p)
{
    pyRefNotNullThrow(p);
}

class PyInitGuard {
public:
    PyInitGuard()
    {
        Py_Initialize();

        PyObject* sysPath = PySys_GetObject((char*)"path");
        auto programName = pyNewRefOrThrow(PyUnicode_FromString("."));
        PyList_Append(sysPath, programName.get());
    }

    ~PyInitGuard()
    {
        Py_Finalize();
    }
};

int callIntFunc(const std::string& proc, int param)
{
    auto pModule = pyNewRefOrThrow(PyImport_ImportModule("PythonCode"));
    auto pDict = pyNewRefOrThrow(PyModule_GetDict(pModule.get()));
    auto pFunc = pyNewRefOrThrow(PyDict_GetItemString(pDict.get(), proc.c_str()));

    if (PyCallable_Check(pFunc.get())) {
        auto pValue = pyNewRef(Py_BuildValue("(i)", param));
        auto pResult = pyNewRef(PyObject_CallObject(pFunc.get(), pValue.get())); // this is wrong tuple is expected here

        return _PyLong_AsInt(pResult.get());
    }
    throw PythonError{};
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
