/*
* Copyright 2020 Virtru Corporation
*
* SPDX - License Identifier: BSD-3-Clause-Clear
*
*/
//
//  TDF SDK
//
//  Created by Sujan Reddy on 2020/04/24
//

#ifndef VIRTRU_TDF_CLIENT_H
#define VIRTRU_TDF_CLIENT_H

#include "attribute_object.h"
#include "entity_object.h"
#include "policy_object.h"
#include "tdf_client_base.h"
#include "oidc_credentials.h"
#include <tdf_constants.h>
#include <unordered_set>

#include <memory>
#include <set>

namespace virtru {

    /// Forward declaration.
    class TDFBuilder;
    class TDF;
    class OIDCService;

    /// A helper class to provide an simple interface for Python bindings. NOT intended for
    /// public API yet. This interface is subject to change.
    class TDFClient : public TDFClientBase {
      public:
        /// DEPRECATED - use OIDC constructors
        /// Constructs a TDF client instance.
        /// \param easUrl - The eas URL.
        /// \param user - The username/email of the user encrypting data.
        TDFClient(const std::string &easUrl, const std::string &user);

        /// Constructs a TDF client instance.
        /// \param backendUrl - The eas URL.
        /// \param user - The registered user on EAS.
        /// \param clientKeyFileName -  Path to client key file.
        /// \param clientCertFileName - Path to client certificate file.
        /// \param sdkConsumerCertAuthority - Path to cert authority file.
        TDFClient(const std::string &backendUrl, const std::string &user, const std::string &clientKeyFileName,
                   const std::string &clientCertFileName, const std::string &sdkConsumerCertAuthority);

        /// Constructor
        /// \param oidcCredentials - OIDC credentials
        /// \param kasUrl -  The KAS backend url
        TDFClient(const OIDCCredentials& oidcCredentials, const std::string &kasUrl);

        /// Default constructor is not supported
        TDFClient() = delete;

        /// Destroy the Client instance.
        ~TDFClient() override;

        /// Copy constructor
        TDFClient(const TDFClient &client) = delete;

        /// Assignment operator
        TDFClient &operator=(const TDFClient &client) = delete;

        /// Move copy constructor
        TDFClient(TDFClient &&client) = delete;

        /// Move assignment operator
        TDFClient &operator=(TDFClient &&client) = delete;

    public: /// Meta data
        /// Assign the metadata that will be encrypted and stored in
        /// the TDF, separately from the data.
        /// \param medata - The metadata that is stored in the tdf
        void setEncryptedMetadata(const std::string& medata);

        /// Decrypt and return TDF metadata as a string. If the TDF content has
        /// no encrypted metadata, will return an empty string.
        /// \param tdfData - The string containing a tdf data.
        /// \return std::string - The string containing the metadata.
        std::string getEncryptedMetadata(const std::string &tdfData);

      public: /// Encrypt and Decrypt
        /// Encrypt the file to tdf format.
        /// \param inFilepath - The file on which the encryption is performed.
        /// \param outFilepath - The file path of the tdf after successful encryption.
        /// NOTE: virtru::exception will be thrown if there is issues while performing the encryption process.
        void encryptFile(const std::string &inFilepath, const std::string &outFilepath) override;

#ifndef SWIG
        /// Encrypt the data to tdf format.
        /// \param plainData - The string containing the data to be encrypted.
        /// \return std::string - The string containing the encrypted data.
        /// NOTE: virtru::exception will be thrown if there are issues while performing the encryption process.
        /// NOTE: The caller should copy the bytes from the return value and should not hold on to the
        /// return value.
        std::string encryptString(const std::string &plainData) override;
#endif

        /// Encrypt the bytes to tdf format.
        /// \param plainData - The vector containing the bytes to be encrypted.
        /// \return std::vector<VBYTE> - The vector containing the encrypted data.
        /// NOTE: virtru::exception will be thrown if there are issues while performing the encryption process.
        /// NOTE: The caller should copy the bytes from the return value and should not hold on to the
        /// return value.
        std::vector<VBYTE> encryptData(const std::vector<VBYTE> &plainData) override;

        /// Decrypt file.
        /// \param inFilepath - The file on which the decryption is performed.
        /// \param outFilepath - The file path of the tdf after successful decryption.
        /// NOTE: virtru::exception will be thrown if there is issues while performing the decryption process.
        void decryptFile(const std::string &inFilepath, const std::string &outFilepath) override;

#ifndef SWIG
        /// Decrypt data from tdf format.
        /// \param encryptedData - The string containing a data to be decrypted.
        /// \return std::string - The string containing the plain data.
        /// NOTE: virtru::exception will be thrown if there are issues while performing the decryption process.
        /// NOTE: The caller should copy the bytes from the return value and should not hold on to the
        /// return value.
        std::string decryptString(const std::string &encryptedData) override;
#endif

        /// Decrypt the bytes from tdf format.
        /// \param encryptedData - The vector containing the bytes to be decrypted.
        /// \return std::vector - The vector containing the plain data.
        /// NOTE: virtru::exception will be thrown if there are issues while performing the decryption process.
        /// NOTE: The caller should copy the bytes from the return value and should not hold on to the
        /// return value.
        std::vector<VBYTE> decryptData(const std::vector<VBYTE> &encryptedData) override;

        // TODO do we need a GetPolicyFile??
        /// Extract and return the JSON policy string from a TDF stream.
        /// \param inStream - The stream containing tdf data.
        /// \param outStream - The stream containing the JSON policy string.
        /// NOTE: virtru::exception will be thrown if there are issues while retrieving the policy.
        std::string getPolicy(const std::string &encryptedData);

        /// Allow user to add data attribute
        /// \param dataAttribute - uri of the attribute
        /// \param kasUrl - kas url
        void addDataAttribute(const std::string &dataAttribute, const std::string &kasURL) override;

        /// Check if file is TDF
        /// \param inFilepath - The file on which the decryption is performed.
        /// \return - Return true if file is TDF and false otherwise
        static bool isFileTDF(const std::string &inFilepath);

        /// Check if string is TDF
        /// \param tdfString - The tdf data on which the check is performed.
        /// \return - Return true if string is TDF and false otherwise
        static bool isStringTDF(const std::string &tdfString);

#ifndef SWIG_JAVA
        /// Check if data is TDF
        /// \param tdfData - The tdf data on which the check is performed.
        /// \return - Return true if data is TDF and false otherwise
        static bool isDataTDF(const std::vector<VBYTE> &tdfData);
#endif

      public:
        /// Create TDFs in XML format instead of zip format.
        void setXMLFormat();

#ifndef SWIG
        /// Set the callback interface which will invoked for all the http network operations.
        /// \param httpServiceProvider - A callback interface which the caller has to implement for performing the
        /// network http operations.
        /// \return - Unique ptr of the TDF instance.
        void setHTTPServiceProvider(std::weak_ptr<INetwork> httpServiceProvider);

#endif

        /// Set the private key(In PEM format), which will be used by this SDK for encryption/decryption of the payload.
        /// \param privateKey - The PEM-encoded private key as a string.
        void setPrivateKey(const std::string& privateKey);

        /// Set the public key(In PEM format), which will be used by this SDK for encryption/decryption of the payload.
        /// \param publicKey - The PEM-encoded public key as a string.
        void setPublicKey(const std::string& publicKey);

      private: /// Helpers
        /// Initialize the TDF builder which is used for creating the TDF instance
        /// used for encrypt and decrypt.
        void initTDFBuilder();

        /// DEPRECATED - OIDC doesn't use entity objects
        /// Get vector of entity attribute objects
        /// \return Return vector of entity attribute objects
        std::vector<AttributeObject> getEntityAttrObjects() override;

        /// Get vector of subject attribute objects
        /// \return Return vector of subject attribute objects
        std::vector<AttributeObject> getSubjectAttrObjects() override;

      private: /// Data
        std::unique_ptr<TDFBuilder> m_tdfBuilder;
        std::unique_ptr<OIDCCredentials> m_oidcCredentials;
        std::unique_ptr<OIDCService> m_oidcService;
    };
} // namespace virtru

#endif //VIRTRU_TDF_CLIENT_H
