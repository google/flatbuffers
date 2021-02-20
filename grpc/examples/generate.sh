current_dir=`pwd`

cd ../..

main_dir=`pwd`

cd ${current_dir}

alias fbc='${main_dir}/Debug/flatc'
generator="--grpc $current_dir/greeter.fbs"

# Regenerate Go lang code
cd go/

cd greeter
fbc --go ${generator}

cd ../..

cd ts/

cd greeter/src
fbc --ts ${generator}
cd ..

cd ../..