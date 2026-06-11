from setuptools import setup
from Cython.Build import cythonize

setup(
    name="fernet",
    ext_modules=cythonize(
        "fernet.pyx",
        language_level=3,
    ),
    zip_safe=False,
)
