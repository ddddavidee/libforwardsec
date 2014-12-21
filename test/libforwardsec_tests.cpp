#include <string>

#include <vector>
#include <stdexcept>
#include <bitset>
#include <assert.h>
#include <cmath>
#include "gmpfse.h"
#include "gtest/gtest.h"

class PFSETests : public ::testing::Test {
protected: 
	 virtual void SetUp(){
	 	test.keygen();
	 	pk = test.pk;
	 } 
	 unsigned int d=3;
	 PFSETests():test(d),testkey(teststring){}
 	 //PairingGroup group;
	 Pfse test;
	 pfsepubkey pk ;
     string teststring = "1111111110101010000011110001001100100111101101110000010001101010100000010110100010111010000100011100011111010100101011011001001110110000110100110011101001110010110001100100111100000111100001101110101000010000001011010110000100100001001001111010001000111001";
	 AESKey testkey;
	// static PairingGroup _group;
};

class BbghhibeTests : public ::testing::Test {
protected: 
     PairingGroup group;

     Bbghibe test;
     BbhHIBEPublicKey pk;

     std::vector<ZR> id0, id1, id00, id01,
                 id10, id11, id111;

     BbghPrivatekey sk0, sk1, sk00, sk01,
                    sk10, sk11, sk111;

     virtual void SetUp(){
        G2 msk;
        test.setup(3,pk,msk);
        ZR z(0);
        ZR o(1);

        id0.push_back(z);
        id1.push_back(o);

        id00.push_back(z);
        id00.push_back(z);

        id01.push_back(z);
        id01.push_back(o);

        id10.push_back(o);
        id10.push_back(z);

        id11.push_back(o);
        id11.push_back(o);

        test.keygen(pk,msk,id0,sk0);
        test.keygen(pk,msk,id1,sk1);

       test.keygen(pk,sk0,id00,sk00);
       test.keygen(pk,sk0,id01,sk01);

       test.keygen(pk,sk1,id10,sk10);
       test.keygen(pk,sk1,id11,sk11);

       id111.push_back(o);
       id111.push_back(o);
       id111.push_back(o);

       test.keygen(pk,sk11,id111,sk111);
     } 


    // static PairingGroup _group;
};



TEST_F(PFSETests,Decrypt){
    vector<string> tags;
    tags.push_back("9");
    PseCipherText ct1 = test.encrypt(pk,testkey,1,tags);

    //test.puncture(1,eight);

    AESKey result = test.decrypt(ct1);
    ASSERT_EQ(testkey,result);
}

TEST_F(PFSETests,FailWhenPunctured){
    vector<string> tags;
    tags.push_back("9");
    PseCipherText ct1 = test.encrypt(pk,testkey,1,tags);
    test.puncture("9");
    //test.puncture(1,eight);
   // test.decrypt(ct1);
   EXPECT_THROW(test.decrypt(ct1),invalid_argument); //FIXME
}

TEST_F(PFSETests,DecryptOnPuncture){
    vector<string> tags;
    tags.push_back("9");

    PseCipherText ct = test.encrypt(pk,testkey,1,tags);

    test.puncture(1,"8");

    AESKey result = test.decrypt(ct);

    EXPECT_EQ(testkey,result);

    // test multiple punctures;
    test.puncture(1,"1");
    test.puncture(1,"2");
    test.puncture(1,"3");

    EXPECT_EQ(testkey,test.decrypt(ct));
	test.prepareNextInterval();

    PseCipherText ct2 = test.encrypt(pk,testkey,2,tags);

    AESKey result1 = test.decrypt(ct2);

    EXPECT_EQ(testkey,result1) ;
}

//
TEST_F(PFSETests,PassOnPunctureNextInterval){
	test.prepareNextInterval();

    vector<string> tags;
    tags.push_back("9");

    test.puncture(2,"8");
    test.puncture(2,"10");

	test.prepareNextInterval();
    PseCipherText ct1 = test.encrypt(pk,testkey,3,tags);

    AESKey result = test.decrypt(ct1);
    EXPECT_EQ(testkey,result);

}
TEST_F(PFSETests,PunctureAndDeriveAll){
	// there are 2^d =1 nodes in a tree of depth d.
	// we don't have the root, so we subtrct one more.
	unsigned int intervals = std::pow(2,d)-2;
	for(unsigned int i =1;i< intervals; i++){
	    vector<string> tags;
	    tags.push_back("9");
	    test.puncture(i,"8");
	    test.puncture(i,"10");
	    test.puncture("11");
	    test.puncture("12");
	    PseCipherText ct1 = test.encrypt(pk,testkey,i,tags);
	    AESKey result = test.decrypt(ct1);
	    EXPECT_EQ(testkey,result);
		test.prepareNextInterval();
	}
}

TEST_F(PFSETests,Delete){

    vector<string> tags;
    tags.push_back("9");

    test.puncture(1,"8");
    test.puncture(1,"10");

    PseCipherText ct = test.encrypt(pk,testkey,1,tags);

    AESKey result = test.decrypt(ct);
    EXPECT_EQ(testkey,result);
    cout << "trying to erase 1" << endl << endl;
    test.eraseKey(1);
    EXPECT_THROW(test.decrypt(ct),invalid_argument); // no key
    EXPECT_THROW(test.eraseKey(2),invalid_argument); // no child keys so count delete
}
TEST_F(PFSETests,PunctureWrongInterval){
    EXPECT_THROW( test.puncture(2,"8");,invalid_argument); // can't puncture key we don't have children for.
}

TEST_F(BbghhibeTests,basic){
    GT m = group.random(GT_t);

    BbghCT ct = test.encrypt(pk,m,id1);
    EXPECT_EQ(m,test.decrypt(sk1, ct));

}
TEST_F(BbghhibeTests,basicFail){
    GT m = group.random(GT_t);

    BbghCT ct = test.encrypt(pk,m,id1);

    EXPECT_NE(m,test.decrypt(sk0,ct));
    EXPECT_NE(m,test.decrypt(sk11,ct));


}
TEST_F(BbghhibeTests,derived){
    GT m = group.random(GT_t);


    BbghCT ct = test.encrypt(pk,m,id11);
    EXPECT_EQ(m,test.decrypt(sk11,ct));
    EXPECT_NE(m,test.decrypt(sk00,ct));
    EXPECT_NE(m,test.decrypt(sk10,ct));
    EXPECT_NE(m,test.decrypt(sk01,ct));

}
TEST_F(BbghhibeTests,derivedFurther){
    GT m = group.random(GT_t);

    BbghCT ct = test.encrypt(pk,m,id111);
    EXPECT_EQ(m,test.decrypt(sk111,ct));
    EXPECT_NE(m,test.decrypt(sk00,ct));
    EXPECT_NE(m,test.decrypt(sk10,ct));
    EXPECT_NE(m,test.decrypt(sk01,ct));

}



// TEST_F(PFSETests,Basic){
//     GT M;
//     M = group.random(GT_t);
//     Pfse test(3);
//     test.keygen();
//     pfsepubkey pk = test.pk;

//     CharmListZR tags;
//      ZR tag = 19;
//      ZR eight = 8;
//      tags.insert(0,tag);
//     CharmList ct1 = test.encrypt(pk,M,1,tags);
//      GT newM = test.decryptGT(ct1);
//      EXPECT_EQ(M,newM);
// }
