docker build --tag 'elektroviz' .
docker run -v "$PWD":/workspace:z 'elektroviz'
mv ./build/Linux/release/elektroviz ./bin/elektroviz.new
