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

cd ${current_dir}

cd swift/

cd Greeter/Sources/Model
fbc --swift ${generator}

cd ${current_dir}

cd ts/

cd greeter/src
fbc --ts ${generator}

cd ${current_dir}