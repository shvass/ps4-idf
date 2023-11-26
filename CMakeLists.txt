idf_component_register(SRCS  "src/ps4_l2cap.c" "src/ps4_parser.c" "src/ps4_spp.c" "src/ps4.c" "src/PS4Controller.cpp"
                    INCLUDE_DIRS "src"
                    REQUIRES bt nvs_flash)
