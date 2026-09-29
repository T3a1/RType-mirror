# syntax=docker/dockerfile:1

# ---------- Build: fully static binary with musl (Alpine) ----------
FROM alpine:3 AS build
RUN apk add --no-cache build-base cmake linux-headers
WORKDIR /app
COPY . .
RUN cmake -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_CLIENT=OFF \
        -DCMAKE_EXE_LINKER_FLAGS="-static -s" \
    && cmake --build build --target r-type_server -j"$(nproc)"

# ---------- Runtime: empty image, just the binary ----------
FROM scratch
COPY --from=build /app/build/r-type_server /r-type_server
USER 65534:65534
EXPOSE 4242/udp
ENTRYPOINT ["/r-type_server"]
CMD ["4242"]
