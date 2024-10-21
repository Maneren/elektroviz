docker build --tag 'elektroviz' .
docker run -v "$PWD":/workspace:z 'elektroviz'
cp ./build/Linux/release/elektroviz ./bin/elektroviz.new
