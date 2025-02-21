#include "iostream"
#include "cmath"
#include <fstream>
#include "utils.h"
#include "species/molecules.h"
#include "species/fragments.h"
#include "species/combined.h"
#include "catch2/catch.hpp"

using namespace std;
using namespace molfunc;


void print_core_xyz(){
    // xyz file appropriate for a core molecule (i.e. a dummy atom that
    // is monovalent)

    ofstream xyz_file ("core.xyz");
    if (xyz_file.is_open()){
        xyz_file << "5\n"
                    "\n"
                    "C          1.57959       -1.40470        0.00000\n"
                    "R          2.68899       -1.40471        0.00000\n"
                    "H          1.20979       -0.63118       -0.70404\n"
                    "H          1.20978       -1.18174        1.02191\n"
                    "H          1.20978       -2.40119       -0.31787\n";
        xyz_file.close();
    }
    else throw runtime_error("Unable to open core.xyz");
}


CoreMolecule core_mol(){
    print_core_xyz();
    CoreMolecule core = CoreMolecule("core.xyz");
    remove("core.xyz");

    return core;
}


CoreMolecule core_mol_two_sites(){

    ofstream xyz_file ("core.xyz");
    if (xyz_file.is_open()){
        xyz_file << "5\n"
                    "\n"
                    "C          1.57959       -1.40470        0.00000\n"
                    "R          2.68899       -1.40471        0.00000\n"
                    "R          1.20979       -0.63118       -0.70404\n"
                    "H          1.20978       -1.18174        1.02191\n"
                    "H          1.20978       -2.40119       -0.31787\n";
        xyz_file.close();
    }
    else throw runtime_error("Unable to open core.xyz");

    CoreMolecule core = CoreMolecule("core.xyz");
    remove("core.xyz");

    return core;
}


CoreMolecule core_pr3(){

    ofstream xyz_file ("core.xyz");
    if (xyz_file.is_open()){
        xyz_file << "4\n"
                    "\n"
                    "P         -1.25349       -0.74286       -0.19277\n"
                    "R          0.16170       -0.56932        0.08583\n"
                    "R         -1.57049        0.65484        0.04481\n"
                    "R         -1.57050       -1.14587        1.16649\n";
        xyz_file.close();
    }
    else throw runtime_error("Unable to open core.xyz");

    CoreMolecule core = CoreMolecule("core.xyz");
    remove("core.xyz");

    return core;
}



CoreMolecule benzene_core_mol(){

    ofstream xyz_file ("core.xyz");
    if (xyz_file.is_open()){
        xyz_file << "12\n"
                    "\n"
                    "C         -3.21403        0.67662       -0.00000\n"
                    "C         -3.20743       -0.72241       -0.00000\n"
                    "C         -2.00574        1.38185       -0.00000\n"
                    "C         -0.79085        0.68805       -0.00000\n"
                    "C         -0.78424       -0.71098       -0.00000\n"
                    "C         -1.99254       -1.41621       -0.00000\n"
                    "H         -1.98743       -2.49853        0.00000\n"
                    "H         -4.14220       -1.26800       -0.00000\n"
                    "H         -4.15391        1.21336       -0.00000\n"
                    "H         -2.01085        2.46417       -0.00000\n"
                    "R          0.14392        1.23364       -0.00000\n"
                    "R          0.15563       -1.24772       -0.00000\n";
        xyz_file.close();
    }
    else throw runtime_error("Unable to open core.xyz");

    CoreMolecule core = CoreMolecule("core.xyz");
    remove("core.xyz");

    return core;
}


TEST_CASE("Test CombinedMolecule init from only a core"){

    auto core = core_mol();
    vector<Fragment> fragments = {};

    REQUIRE_NOTHROW(CombinedMolecule(core, fragments));
}


TEST_CASE("Test throws on unequal fragments and dummy core atoms"){

    auto core = core_mol();
    vector<Fragment> fragments = {FragmentLib::instance().fragment("Br"),
                                  FragmentLib::instance().fragment("Br")};

    // One dummy atom in the core but two fragments
    REQUIRE_THROWS(CombinedMolecule(core, fragments));
}


TEST_CASE("Test simple H3CBr combined construction") {

    auto core = core_mol();
    vector<Fragment> fragments = {FragmentLib::instance().fragment("Br")};

    auto mol = CombinedMolecule(core, fragments).to_molecule();
    int br_idx = 4;

    // Ensure the C-Br distance is reasonable
    REQUIRE((mol.distance(0, br_idx) > 1.5 && mol.distance(0, br_idx) < 2.5));

    // and that there are no short Br-H contacts
    auto h_atom_idxs = vector<int>{1, 2, 3};
    for (auto idx: h_atom_idxs){
        REQUIRE((mol.distance(idx, br_idx) > 2.0));   // r(Br-H) > 2.0 Å
    }
}


TEST_CASE("Test simple repulsive energy"){

    auto mol = CombinedMolecule(core_mol(), {});
    auto fragment = FragmentLib::instance().fragment("Br");

    // Place the fragment in a specific location
    fragment.coordinates[0] = {3.539590, -1.404700,	-0.000018};
    mol.fragments = {fragment};

    // Built molecule should have a lower repulsion than a close translation
    // of the fragment
    double rep_e = mol.repulsive_energy();

    mol.fragments[0].translate({-0.1, 0.0, 0.0});

    REQUIRE(rep_e < mol.repulsive_energy());
    rep_e = mol.repulsive_energy();

    // and if it's translated even closer
    mol.fragments[0].translate({-0.3, 0.0, 0.0});
    REQUIRE(rep_e < mol.repulsive_energy());

}


TEST_CASE("Test simple ethane combined construction") {

    auto core = core_mol();
    vector<Fragment> fragments = {FragmentLib::instance().fragment("Me")};

    auto mol = CombinedMolecule(core, fragments);

    REQUIRE(mol.repulsive_energy() < 7);

    // Check the new carbon-carbon distance is reasonable
    auto full_mol = mol.to_molecule();
    REQUIRE(full_mol.n_atoms() == 8);

    for (int atom_idx=1; atom_idx<full_mol.n_atoms(); atom_idx++){

        if (full_mol.atoms[atom_idx].symbol != "C") continue;

        REQUIRE(utils::is_close(full_mol.distance(0, atom_idx),
                                1.5,       // r^0(C-C) ~ 1.5 Å
                                0.2));     // absolute tolerance (Å)
    }
}


TEST_CASE("Test simple propane combined construction") {

    auto core = core_mol_two_sites();
    vector<Fragment> fragments = {FragmentLib::instance().fragment("Me"),
                                  FragmentLib::instance().fragment("Me")};

    auto mol = CombinedMolecule(core, fragments);
    REQUIRE(mol.repulsive_energy() < 10);
}


TEST_CASE("Test o-ditertbutylbenzene combined construction"){
    auto core = benzene_core_mol();
    vector<Fragment> fragments = {FragmentLib::instance().fragment("tBu"),
                                  FragmentLib::instance().fragment("tBu")};

    auto mol = CombinedMolecule(core, fragments);
    REQUIRE(mol.repulsive_energy() < 30);
}


TEST_CASE("Test angle potentials"){

    // Not a very good geometry OH fragment..
    auto fragment = Fragment({Atom3D("O", 0.0, 0.0, 0.0),
                              Atom3D("R", -1.0, 0.0, 0.0),
                              Atom3D("H", 1.0, 0.0, 0.0)},
                             {"hydroxyl"});


    auto mol = CombinedMolecule(core_mol(),
                                {fragment});

     mol.gen_angle_potentials();

    REQUIRE(mol.angle_potentials.size() == 1);

    REQUIRE(mol.angle_potentials[0].atom_idxs.size() == 3);

    // Indexing is without dummy atoms so the C-O-H angle
    // should have indexes 0-4-5
    REQUIRE(mol.angle_potentials[0].atom_idxs[0] == 0);
    REQUIRE(mol.angle_potentials[0].atom_idxs[1] == 4);
    REQUIRE(mol.angle_potentials[0].atom_idxs[2] == 5);

    // Ensure the value can be calculated and is just positive
    auto coords = mol.to_molecule().coordinates;
    REQUIRE(mol.angle_potentials.value(coords) > 0);
}


TEST_CASE("Test angle potential value"){

    auto mol = CombinedMolecule(core_mol(),
                                {FragmentLib::instance().fragment("OH")});

    // Should have a single angle potential for the C-O-H
    REQUIRE(mol.angle_potentials.size() == 1);

    // Check indexing, with OH at the end of the molecule
    REQUIRE(mol.to_molecule().n_atoms() == 6);
    REQUIRE(mol.to_molecule().atoms[4].symbol == "O");
    REQUIRE(mol.to_molecule().atoms[5].symbol == "H");

    vector<Coordinate> bent_coords = {{1.50280, -1.40472, -0.00011},
                                      {1.14970, -1.18053,  1.02747},
                                      {1.11401, -2.40488, -0.29029},
                                      {1.11403, -0.61632, -0.68052},
                                      {2.90096, -1.40649, -0.00816},
                                      {3.16817, -1.61021, -0.94189}};

    vector<Coordinate> lin_coords = {{1.50280, -1.40472, -0.00011},
                                     {1.14970, -1.18053,  1.02747},
                                     {1.11401, -2.40488, -0.29029},
                                     {1.11403, -0.61632, -0.68052},
                                     {2.90096, -1.40649, -0.00816},
                                     {4.00180, -1.37777, -0.22074}};

    // with an angle potential for the C-O-H the bent coordinates
    // should be lower in energy
    REQUIRE(mol.angle_potentials.value(bent_coords)
            < mol.angle_potentials.value(lin_coords));
}


TEST_CASE("Test total energy is not none"){

    auto mol = CombinedMolecule(core_mol(),
                                {FragmentLib::instance().fragment("OH")});
    auto coords = mol.coordinates();

    REQUIRE(mol.total_energy(coords) > 0);

    // Likewise if the angle potentials have been generated
    mol.gen_angle_potentials();
    REQUIRE(mol.total_energy(coords) > 0);

    REQUIRE(mol.total_energy(coords) != NAN);
}


TEST_CASE("Test fragment atom indexing one fragment"){

    auto frag = Fragment({Atom3D("S", 0.0, 0.0, 0.0),
                          Atom3D("R", -1.0, 00, 0.0),
                          Atom3D("H", 1.0, 0.0, 0.0)},
                         {"SH"});

    auto mol = CombinedMolecule(core_mol(), {frag});

    REQUIRE(mol.fragment_origin_idxs.size() == 1);
    REQUIRE(mol.fragment_origin_idxs[0] == 4);     // First atom after the core

    REQUIRE(mol.fragments_atom_idxs.size() == 1);
    // Should only have the two atoms of the fragment (S, H)
    REQUIRE(mol.fragments_atom_idxs[0].size() == 2);
    REQUIRE(mol.fragments_atom_idxs[0][0] == 4);
    REQUIRE(mol.fragments_atom_idxs[0][1] == 5);
}


TEST_CASE("Test fragment atom indexing two fragments-"){

    auto frag = Fragment({Atom3D("S", 0.0, 0.0, 0.0),
                          Atom3D("R", -1.0, 00, 0.0),
                          Atom3D("H", 1.0, 0.0, 0.0)},
                         {"SH"});

    auto mol = CombinedMolecule(core_mol_two_sites(),
                                {frag, Fragment(frag)});
    mol.gen_fragment_idxs();

    REQUIRE(mol.fragment_origin_idxs.size() == 2);
    REQUIRE(mol.fragment_origin_idxs[0] == 3);     // First atom after the core
    REQUIRE(mol.fragment_origin_idxs[1] == 5);

    REQUIRE(mol.fragments_atom_idxs.size() == 2);

    // Should only have the two atoms of the fragment (S, H)
    REQUIRE(mol.fragments_atom_idxs[1].size() == 2);
    REQUIRE(mol.fragments_atom_idxs[1][0] == 5);
    REQUIRE(mol.fragments_atom_idxs[1][1] == 6);
}


TEST_CASE("Test underdefined number of fragments"){

    auto mol = CombinedMolecule(core_mol_two_sites(),
                                {FragmentLib::instance().fragment("Br")});

    // should not throw an exception and have the correct number of atoms
    REQUIRE(mol.to_molecule().n_atoms() == 5);

    // and a distance between the two that means the fragment
    // has been copied
    REQUIRE(mol.to_molecule().distance(3, 4) > 1.0);
}


TEST_CASE("Test PMe3"){

    auto me = FragmentLib::instance().fragment("Me");
    auto combined = CombinedMolecule(core_pr3(), {me});
    auto coords = combined.coordinates();

    REQUIRE(combined.total_energy(coords) < 10);
}


TEST_CASE("Test methane + Me, F"){

    auto f = FragmentLib::instance().fragment("F");
    auto me = FragmentLib::instance().fragment("Me");

    // Test the coordinates are all valid in the construction
    auto mol = CombinedMolecule(core_mol_two_sites(),
                                {f, me});

    for (auto &coord : mol.coordinates()){
        REQUIRE_FALSE(isnan(coord.x()));
        REQUIRE_FALSE(isnan(coord.y()));
        REQUIRE_FALSE(isnan(coord.z()));
    }
}

