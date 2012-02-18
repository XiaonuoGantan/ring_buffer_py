from setuptools import (
    setup,
    find_packages,
)
from distutils.core import Extension

buffer_m = Extension('ring_buffer',
                     sources = ['type_extension.c', 'src/buffer.c'],
                     include_dirs = ['src/'],
                     extra_compile_args = ['-g'],
                    )

setup(name = 'ring_buffer',
      version = '1.0',
      description = '''This is a Python wrapper for a Optimized POSIX
      C Implementation: http://en.wikipedia.org/wiki/Ring_buffer.''',
      classifiers = [
          "Programming Language :: Python :: C",
      ],
      author = 'Xiaonuo Gantan',
      author_email = 'xiaonuo.gantan@gmail.com',
      keywords='python c extension circular buffer mmap',
      packages=find_packages('.'),
      include_package_data=True,
      ext_modules = [buffer_m],
      install_requires = [
          'nose',
      ],
     )
