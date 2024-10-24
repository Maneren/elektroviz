(
  cd src || exit 1
  docker build --tag 'elektroviz' .
  docker run -v "$PWD":/workspace:z -u "$(id -u "$USER"):$(id -g "$USER")" 'elektroviz'
)
cp ./src/build/Linux/release/elektroviz ./bin/elektroviz.new
cp -r ./src/scenarios ./
