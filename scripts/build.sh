docker build --tag 'elektroviz' .
docker run -v "$PWD":/workspace:z -u "$(id -u "$USER"):$(id -g "$USER")" 'elektroviz'
cp ./build/Linux/release/elektroviz ./bin/elektroviz.new
